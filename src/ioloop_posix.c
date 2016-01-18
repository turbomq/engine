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
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include "engine.h"
#include "ioloop.h"
#include "utils.h"
#include "linked_list.h"

typedef struct
{
    turbo_ioloop_t* ioloop;
    pthread_t thread_id;
    turbo_linked_list_t* clients;
} thread_info_t;

typedef struct 
{
    turbo_ioloop_t parent;
    turbo_engine_t* engine;
    char* host;
    int port;    
    int num_threads;
    int socketfd;
    int keep_going;
    thread_info_t* thread_infos;
    pthread_t main_loop_thread_id;
    on_new_connection_func_t on_new_connection_func;
} turbo_ioloop_ex_t;

typedef struct
{
    turbo_remote_client_t remote_client;
    short events;
    int index;
} posix_client_t;

void turbo_ioloop_destroy_message(void* data)
{
    turbo_message_t* message = (turbo_message_t*)data;
    if(message != NULL)
    {
        turbo_message_destroy(&message);
    }
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
    ioloop->thread_infos = NULL;
    ioloop->on_new_connection_func = NULL;
    if(turbo_ioloop_create_socket(ioloop) == -1)
    {
        free(ioloop);
        return NULL;
    }
    return (turbo_ioloop_t*)ioloop;
}

void posix_client_release(void* data)
{
    posix_client_t* posix_client = (posix_client_t*)data;
    turbo_remote_client_t* client = &posix_client->remote_client;
    /*
     * Cleans the output stream up if it is not released yet.
     */
    if(client->out_stream != NULL)
    {
        turbo_out_stream_destroy(&client->out_stream);
    }
    
    /*
     * Cleans the input stream up if it is not released yet.
     */
    if(client->in_stream != NULL)
    {
        turbo_in_stream_destroy(&client->in_stream);
    }
        
    /*
     * Closes the socket.
     */
    close(client->socketfd);
    
    /*
     * Frees the allocated memory
     */
    free(posix_client);
}

int turbo_ioloop_client_close(turbo_ioloop_ex_t* ioloop, thread_info_t* thread_info, posix_client_t* posix_client)
{
    /*
     * Remove client from the process list
     */
    turbo_linked_list_pop(thread_info->clients, posix_client->index);

    posix_client_release(posix_client);

    return 0;
}

int turbo_ioloop_client_connect(turbo_ioloop_ex_t* ioloop, thread_info_t* thread_info)
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
    posix_client_t* client = (posix_client_t*)malloc(sizeof(posix_client_t));
    client->remote_client.socketfd = client_socketfd;
    client->remote_client.ip = 0;
    client->remote_client.in_stream = NULL;
    client->remote_client.out_stream = NULL;
    client->events = POLLIN;
    client->index = turbo_linked_list_size(thread_info->clients);

    turbo_linked_list_append(thread_info->clients, client);

    /*
     * Triggers engine by sending the new connection event.
     */
    if(ioloop->on_new_connection_func != NULL)
    {
        if (ioloop->on_new_connection_func((turbo_ioloop_t*)ioloop, &client->remote_client) == -1)
        {
            turbo_ioloop_client_close(ioloop, thread_info, client);
            return -1;
        }
    }

    return 0;
}

int turbo_ioloop_client_read(turbo_ioloop_ex_t* ioloop, thread_info_t* thread_info, posix_client_t* posix_client)
{
    size_t loaded;
    int64_t command;
    int result;
    
    /*
     * Get remote client information.
     */
    turbo_remote_client_t* client = &posix_client->remote_client;
        
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
        posix_client->events = POLLOUT;
    }

    turbo_in_stream_destroy(&client->in_stream);

    return result;
}

int turbo_ioloop_client_write(turbo_ioloop_ex_t* ioloop, thread_info_t* thread_info, posix_client_t* posix_client)
{
    turbo_remote_client_t* client = &posix_client->remote_client;
    size_t result;
    if(client->out_stream != NULL)
    {
        result = turbo_out_stream_send(client->out_stream);
        if(result > 0)
        {
            turbo_out_stream_destroy(&client->out_stream);
            posix_client->events = POLLIN;
            return 0;
        }
        return result;
    }
    return -1;
}

void* turbo_ioloop_main_loop(void* data)
{
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)data;
    const short EVENT_TIMEOUT = 1000;
    short result;
    short thread_counter = 0;
    struct pollfd event;
    
    event.fd = ioloop->socketfd;
    event.events = POLLIN;
    
    while(ioloop->keep_going)
    {
        result = poll(&event, 1, EVENT_TIMEOUT);
        if(result == -1)
        {
            return NULL;
        }
        if(event.revents & POLLIN)
        {
            if(thread_counter >= ioloop->num_threads)
            {
                thread_counter = 0;
            }
            turbo_ioloop_client_connect(ioloop, &ioloop->thread_infos[thread_counter]);
            thread_counter += 1;
        }
    }
    
    return NULL;
}

void* turbo_ioloop_child_loop(void* base)
{
    thread_info_t* info = (thread_info_t*)base;
    turbo_ioloop_ex_t* ioloop = (turbo_ioloop_ex_t*)info->ioloop;
    const short EVENT_TIMEOUT = 5000;
    int number_of_events;
    int index;
    int total_clients;
    struct pollfd* events = NULL;
    struct pollfd* current;
    posix_client_t* client;
    
    while(ioloop->keep_going)
    {
        total_clients = turbo_linked_list_size(info->clients);
        
        if(total_clients == 0)
        {
            sleep(1);
            continue;
        }

        /*
         * Collects clients to get events.
         */
        events = realloc(events, total_clients * sizeof(struct pollfd));
        for(index = 0; index < total_clients; index++) 
        {
            client = (posix_client_t*)turbo_linked_list_get(info->clients, index);
            events[index].fd = client->remote_client.socketfd;
            events[index].events = client->events;
        }
        
        /*
         * Gets the available events to process.
         */
        number_of_events = poll(events, total_clients, EVENT_TIMEOUT);
        
        /*
         * If there is a error in POLL then terminates the IO loop.
         */
        if(number_of_events == -1)
        {
            ioloop->keep_going = 0;
        }
        
        /*
         * Processes the events
         */
        index = 0;
        while(index < total_clients)
        {
            posix_client_t* client = turbo_linked_list_get(info->clients, index);
            current = &events[index];
            if(current->revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                turbo_ioloop_client_close(ioloop, info, client);
                total_clients -= 1;
                continue;
            }
            else if(current->revents & POLLIN)
            {
                if(turbo_ioloop_client_read(ioloop, info, client) == -1)
                {
                    print_system_error();
                    turbo_ioloop_client_close(ioloop, info, client);
                    total_clients -= 1;
                    continue;
                }
            }
            else if(current->revents & POLLOUT)
            {
                if(turbo_ioloop_client_write(ioloop, info, client) == -1)
                {
                    turbo_ioloop_client_close(ioloop, info, client);
                    total_clients -= 1;
                    continue;
                }
            }
            index += 1;
        }
    }
    if(events != NULL)
    {
        free(events);
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
    
    ioloop->thread_infos = (thread_info_t*)malloc(sizeof(thread_info_t) * ioloop->num_threads);
    for(index = 0; index < ioloop->num_threads; index++)
    {
        thread_info_t* info = &ioloop->thread_infos[index];
        info->clients = turbo_linked_list_create(posix_client_release);
        info->ioloop = base;
        if(pthread_create(&info->thread_id, 
            NULL,
            turbo_ioloop_child_loop,
            info) != 0)
        {
            turbo_linked_list_destroy(&info->clients);
            free(ioloop->thread_infos);
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
        if(ptr->thread_infos != NULL)
        {
            for(index = 0; index < ptr->num_threads; index++)
            {
                pthread_join(ptr->thread_infos[index].thread_id, &thread_return_value);
                turbo_linked_list_destroy(&ptr->thread_infos[index].clients);
            }
            free(ptr->thread_infos);
        }
        free(ptr->host);
        close(ptr->socketfd);
        free(ptr);
        *base = NULL;
    }
    return 0;
}