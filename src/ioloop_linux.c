/*
 *    Copyright (C) 2015 abi <abisxir@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "engine.h"
#include "ioloop.h"
#include "utils.h"

typedef struct 
{
    turbo_ioloop_t parent;
    turbo_engine_t* engine;
    char* host;
    int port;    
    int num_threads;
    int socketfd;
    int epollfd;
    int keep_going;
    int* child_epollfds;
    pthread_t* child_thread_ids;
    pthread_t main_loop_thread_id;
    on_new_connection_func_t on_new_connection_func;
} turbo_ioloop_ex_t;

typedef struct
{
    turbo_ioloop_ex_t* ioloop;
    int epollfd;
} thread_info_t;

void turbo_ioloop_destroy_message(void* data)
{
    turbo_message_t* message = (turbo_message_t*)data;
    if(message != NULL)
    {
        turbo_message_destroy(&message);
    }
}

int epoll_modify(int epollfd, int operation, int fd, uint32_t events, void* data)
{
    struct epoll_event server_listen_event;
    server_listen_event.events = events;
    server_listen_event.data.ptr = data;
    return epoll_ctl(epollfd, operation, fd, &server_listen_event);
}

int turbo_ioloop_create_socket(turbo_ioloop_ex_t* ioloop)
{
    int socketfd = create_socket(ioloop->host, ioloop->port);
    if(socketfd == -1)
    {
        return -1;
    }
    ioloop->socketfd = socketfd;
    return 0;
}

int turbo_ioloop_create_epoll(turbo_ioloop_ex_t* ioloop)
{
    ioloop->epollfd = epoll_create1(0);
    if(ioloop->epollfd == -1)
    {
        return -1;
    }
    return epoll_modify(ioloop->epollfd, 
                        EPOLL_CTL_ADD, 
                        ioloop->socketfd, 
                        EPOLLIN, 
                        &ioloop->socketfd);
}

turbo_ioloop_t* turbo_ioloop_create(turbo_engine_t* engine, 
                                    const char* protocol, 
                                    const char* host, 
                                    int port, 
                                    int num_threads)
{
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)malloc(sizeof(turbo_ioloop_ex_t));
    ioloop->engine = engine;
    ioloop->host = strdup(host);
    ioloop->port = port;
    ioloop->num_threads = num_threads;
    ioloop->keep_going = 0;
    ioloop->child_thread_ids = NULL;
    ioloop->child_epollfds = NULL;
    ioloop->on_new_connection_func = NULL;
    if(turbo_ioloop_create_socket(ioloop) == -1)
    {
        free(ioloop);
        return NULL;
    }
    if(turbo_ioloop_create_epoll(ioloop) == -1)
    {
        close(ioloop->socketfd);
        free(ioloop);
        return NULL;
    }
    return (turbo_ioloop_t*)ioloop;
}

int turbo_ioloop_client_close(turbo_ioloop_ex_t* ioloop, int epollfd, turbo_remote_client_t* client)
{
    if(client->out_stream != NULL)
    {
        turbo_out_stream_destroy(&client->out_stream);
    }
    if(client->in_stream != NULL)
    {
        turbo_in_stream_destroy(&client->in_stream);
    }
    epoll_modify(epollfd, EPOLL_CTL_DEL, client->socketfd, EPOLLIN, client);
    close(client->socketfd);
    free(client);
    return 0;
}

int turbo_ioloop_client_connect(turbo_ioloop_ex_t* ioloop, int epollfd)
{
    struct sockaddr_in addr;
    socklen_t len = (socklen_t)sizeof(addr);
    
    /*
     * Accepts client but the final accept is depends on the engine.
     */
    int client_socketfd = accept(ioloop->socketfd, (struct sockaddr*)&addr, &len);

    if(client_socketfd == -1)
    {
        return -1;
    }

    /*
     * Makes the socket non-blocking.
     */
    make_socket_non_blocking(client_socketfd);
    
    /*
     * Collects new client information and registers it to handle read event.
     */
    turbo_remote_client_t* client = (turbo_remote_client_t*)malloc(sizeof(turbo_remote_client_t));
    client->socketfd = client_socketfd;
    client->ip = 0;
    client->in_stream = NULL;
    client->out_stream = NULL;

    if(epoll_modify(epollfd, EPOLL_CTL_ADD, client_socketfd, EPOLLIN, client) == -1)
    {
        free(client);
        close(client_socketfd);
        return -1;
    }

    /*
     * Triggers engine by sending the new connection event.
     */
    if(ioloop->on_new_connection_func != NULL)
    {
        if (ioloop->on_new_connection_func((turbo_ioloop_t*)ioloop, client) == -1)
        {
            turbo_ioloop_client_close(ioloop, epollfd, client);
            return -1;
        }
    }

    return 0;
}

int turbo_ioloop_client_read(turbo_ioloop_ex_t* ioloop, int epollfd, turbo_remote_client_t* client)
{
    size_t loaded;
    int64_t command;
    int result;
        
    if(client->in_stream == NULL)
    {
        client->in_stream = turbo_in_stream_create(client->socketfd);
        if(client->in_stream == NULL)
        {
            print_system_error();
            return -1;
        }
    }
    
    loaded = turbo_in_stream_recv(client->in_stream);
    
    if(loaded <= 0)
    {
        return loaded;
    }
    
    if(turbo_in_stream_read_int64(client->in_stream, &command) == -1)
    {
        turbo_in_stream_destroy(&client->in_stream);
        return -1;
    }

    result = turbo_engine_execute(ioloop->engine, command, client);
    
    if(client->out_stream != NULL)
    {
        epoll_modify(epollfd, EPOLL_CTL_DEL, client->socketfd, EPOLLIN, client);
        epoll_modify(epollfd, EPOLL_CTL_ADD, client->socketfd, EPOLLOUT, client);
    }

    turbo_in_stream_destroy(&client->in_stream);

    return result;
}

int turbo_ioloop_client_write(turbo_ioloop_ex_t* ioloop, int epollfd, turbo_remote_client_t* client)
{
    size_t result;
    if(client->out_stream != NULL)
    {
        result = turbo_out_stream_send(client->out_stream);
        if(result > 0)
        {
            turbo_out_stream_destroy(&client->out_stream);
            if(epoll_modify(epollfd, EPOLL_CTL_DEL, client->socketfd, EPOLLOUT, client) == -1)
            {
                return -1;
            }
            return epoll_modify(epollfd, EPOLL_CTL_ADD, client->socketfd, EPOLLIN, client);
        }
        return result;
    }
    return -1;
}

void* turbo_ioloop_main_loop(void* data)
{
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)data;
    const short MAX_EVENTS = 64;
    const short EVENT_TIMEOUT = 1000;
    short number_of_events;
    short index;
    short epoll_counter = 0;
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event* current;
    
    while(ioloop->keep_going)
    {
        number_of_events = epoll_wait(ioloop->epollfd, events, MAX_EVENTS, EVENT_TIMEOUT);
        if(number_of_events == -1)
        {
            return NULL;
        }
        for(index = 0; index < number_of_events; index++)
        {
            current = &events[index];
            if(current->data.ptr == &ioloop->socketfd)
            {
                if(epoll_counter >= ioloop->num_threads)
                {
                    epoll_counter = 0;
                }
                //turbo_ioloop_client_connect(ioloop, ioloop->epollfd);
                turbo_ioloop_client_connect(ioloop, ioloop->child_epollfds[epoll_counter]);
                epoll_counter += 1;
            }
        }
    }
    
    return NULL;
}

void* turbo_ioloop_child_loop(void* base)
{
    thread_info_t* info = (thread_info_t*)base;
    turbo_ioloop_ex_t* ioloop = info->ioloop;
    int epollfd = info->epollfd;
    free(info);
    const short MAX_EVENTS = 64;
    const short EVENT_TIMEOUT = 1000;
    short number_of_events;
    short index;
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event* current;
    
    while(ioloop->keep_going)
    {
        number_of_events = epoll_wait(epollfd, events, MAX_EVENTS, EVENT_TIMEOUT);
        if(number_of_events == -1)
        {
            ioloop->keep_going = 0;
        }
        for(index = 0; index < number_of_events; index++)
        {
            current = &events[index];
            if(current->events & EPOLLHUP || current->events & EPOLLERR)
            {
                turbo_remote_client_t* client = (turbo_remote_client_t*)current->data.ptr;
                turbo_ioloop_client_close(ioloop, epollfd, client);
            }
            else if(current->events & EPOLLIN)
            {
                turbo_remote_client_t* client = (turbo_remote_client_t*)current->data.ptr;
                if(turbo_ioloop_client_read(ioloop, epollfd, client) == -1)
                {
                    print_system_error();
                    turbo_ioloop_client_close(ioloop, epollfd, client);
                }
            }
            else if(current->events & EPOLLOUT)
            {
                turbo_remote_client_t* client = (turbo_remote_client_t*)current->data.ptr;
                if(turbo_ioloop_client_write(ioloop, epollfd, client) == -1)
                {
                    turbo_ioloop_client_close(ioloop, epollfd, client);
                }
            }
            else
            {
                //printf("UNHANDLED EVENT!!!!!\n");
            }
        }
    }
    return NULL;
}


int turbo_ioloop_run(turbo_ioloop_t* base)
{
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)base;
    int index;
    if(ioloop->keep_going)
    {
        return 0;
    }
    
    ioloop->keep_going = 1;

    /*
     * Starts main loop in a seperated thread.
     */
    if(pthread_create(&ioloop->main_loop_thread_id, 
        NULL,
        turbo_ioloop_main_loop,
        ioloop) != 0)
    {
        return -1;
    }
    
    /*
     * Initiates the child epoll handlers
     */
    ioloop->child_epollfds = (int*)malloc(sizeof(int) * ioloop->num_threads);
    for(index = 0; index < ioloop->num_threads; index++)
    {
        ioloop->child_epollfds[index] = epoll_create1(0);
        if(ioloop->child_epollfds[index] == -1)
        {
            free(ioloop->child_epollfds);
            ioloop->child_epollfds = NULL;
            return -1;
        }
    }

    ioloop->child_thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * ioloop->num_threads);
    for(index = 0; index < ioloop->num_threads; index++)
    {
        thread_info_t* info = malloc(sizeof(thread_info_t));
        info->ioloop = ioloop;
        info->epollfd = ioloop->child_epollfds[index];
        if(pthread_create(&ioloop->child_thread_ids[index], 
            NULL,
            turbo_ioloop_child_loop,
            info) != 0)
        {
            free(info);
            free(ioloop->child_thread_ids);
            ioloop->child_thread_ids = NULL;
            return -1;
        }
    }

    return 0;
}

/*
 * Stops the given IO loop.
 */
int turbo_ioloop_stop(turbo_ioloop_t* base)
{
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)base;
    ioloop->keep_going = 0;
    return 0;
}

int turbo_ioloop_destroy(turbo_ioloop_t** base)
{
    void* thread_return_value;
    int index;
    turbo_ioloop_ex_t* ptr = (turbo_ioloop_ex_t*)*base;
    
    if(ptr != NULL)
    {
        ptr->keep_going = 0;
        if(ptr->child_thread_ids != NULL)
        {
            for(index = 0; index < ptr->num_threads; index++)
            {
                pthread_join(ptr->child_thread_ids[index], &thread_return_value);
            }
            free(ptr->child_thread_ids);
        }
        if(ptr->child_epollfds != NULL)
        {
            for(index = 0; index < ptr->num_threads; index++)
            {
                close(ptr->child_epollfds[index]);
            }
            free(ptr->child_epollfds);
        }
        free(ptr->host);
        close(ptr->epollfd);
        close(ptr->socketfd);
        free(ptr);
        *base = NULL;
    }
    return 0;
}