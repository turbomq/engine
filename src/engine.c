/*
 *    Copyright (C) 2015 abi <abi@singiro.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "ioloop.h"
#include "engine.h"
#include "hashmap.h"
#include "utils.h"
#include "ioloop.h"
#include "array_list.h"
#include "command.h"

typedef struct
{
    turbo_engine_t parent;
    turbo_hashmap_t* queues;
    turbo_array_list_t* commands;
    turbo_ioloop_t* ioloop;
    pthread_mutex_t lock;
} turbo_engine_ex_t;

void engine_release_command(void* ptr)
{
    turbo_command_t* command = (turbo_command_t*)ptr;
    turbo_command_destroy(&command);
}

int engine_command_push(turbo_command_t* command, turbo_remote_client_t* client)
{
    char qname[256];
    char topic[256];
    void* content;
    int size;
    turbo_message_t* message;
    turbo_queue_t* queue;
    turbo_engine_t* engine = turbo_command_get_context(command);

    turbo_in_stream_read_str(client->in_stream, qname);
    turbo_in_stream_read_str(client->in_stream, topic);
    turbo_in_stream_read_int32(client->in_stream, &size);
    content = malloc(size);
    if(content == NULL)
    {
        printf("!!!Can not allocate memory to receive content.\n");
        print_system_error();
        return -1;
    }
    turbo_in_stream_read(client->in_stream, content, size);
    queue = turbo_engine_get_queue(engine, qname);
    message = turbo_message_create_ex(client->ip, content, size);
    turbo_queue_push(queue, topic, message);

    return 0;
}

int engine_command_pop(turbo_command_t* command, turbo_remote_client_t* client)
{
    char qname[256];
    char topic[256];
    int8_t timeout;
    turbo_message_t* message;
    turbo_queue_t* queue;
    int result;
    turbo_engine_t* engine = turbo_command_get_context(command);

    turbo_in_stream_read_str(client->in_stream, qname);
    turbo_in_stream_read_str(client->in_stream, topic);
    turbo_in_stream_read(client->in_stream, &timeout, sizeof(timeout));

    queue = turbo_engine_get_queue(engine, qname);
    message = turbo_queue_pop(queue, topic, timeout);

    if(client->out_stream != NULL)
    {
        turbo_out_stream_destroy(&client->out_stream);
    }

    client->out_stream = turbo_out_stream_create(client->socketfd);
    if(client->out_stream == NULL)
    {
        print_system_error();
        return -1;
    }

    result = turbo_out_stream_append_message(client->out_stream, message);

    turbo_message_destroy(&message);

    if(result == -1)
    {
        print_system_error();
        return result;
    }

    return 0;
}

/*
 * Creates a instance of an engine. Protocol can be ssl or tcp at the moment.
 */
turbo_engine_t* turbo_engine_create(const char* protocol, const char* host, int port, int num_threads)
{
    turbo_engine_ex_t* engine = (turbo_engine_ex_t*)malloc(sizeof(turbo_engine_ex_t));

    if(num_threads == 0)
    {
        //num_threads = pthread_num_processors_np() * 4;
        num_threads = 8;
    }
    engine->ioloop = turbo_ioloop_create((turbo_engine_t*)engine, protocol, host, port, num_threads);
    engine->queues = turbo_hashmap_str_create();
    engine->commands = turbo_array_list_create(engine_release_command);

    /*
     * Adding push command
     */
    turbo_array_list_append(engine->commands, turbo_command_create(0, engine_command_push, engine));

    /*
     * Adding pop command
     */
    turbo_array_list_append(engine->commands, turbo_command_create(1, engine_command_pop, engine));

    pthread_mutex_init(&engine->lock, NULL);
    return (turbo_engine_t*)engine;
}

/*
 * Runs the queue engine.
 */
int turbo_engine_run(turbo_engine_t* base)
{
    turbo_engine_ex_t* engine = (turbo_engine_ex_t*)base;
    return turbo_ioloop_run(engine->ioloop);
}

/*
 * Executes a command.
 */
int turbo_engine_execute(turbo_engine_t* base, int64_t key, void* remote_client)
{
    turbo_remote_client_t* client = (turbo_remote_client_t*)remote_client;
    turbo_engine_ex_t* engine = (turbo_engine_ex_t*)base;
    int i;
    for(i = 0; i < turbo_array_list_size(engine->commands); i++)
    {
        turbo_command_t* command = (turbo_command_t*)turbo_array_list_get(engine->commands, i);
        if(turbo_command_get_key(command) == key)
        {
            return turbo_command_handle(command, client);
        }
    }
    return -1;
}

void turbo_queue_release(void* value)
{
    turbo_queue_t* queue = (turbo_queue_t*)value;
    if(queue != NULL)
    {
        turbo_queue_destroy(&queue);
    }
}

/*
 * Stops turbo engine.
 */
int turbo_engine_stop(turbo_engine_t* base)
{
    turbo_engine_ex_t* engine = (turbo_engine_ex_t*)base;
    return turbo_ioloop_stop(engine->ioloop);
}

/*
 * Destroys the given engine.
 */
void turbo_engine_destroy(turbo_engine_t** engine)
{
    turbo_engine_ex_t* ptr = (turbo_engine_ex_t*)*engine;
    if(ptr != NULL)
    {
        turbo_ioloop_destroy(&ptr->ioloop);
        turbo_hashmap_destroy(&ptr->queues, free, turbo_queue_release);
        turbo_array_list_destroy(&ptr->commands);
        pthread_mutex_destroy(&ptr->lock);
        free(ptr);
        *engine = NULL;
    }
}

/*
 * Declares a queue.
 */
turbo_queue_t* turbo_engine_get_queue(turbo_engine_t* base, const char* name)
{
    turbo_engine_ex_t* engine = (turbo_engine_ex_t*)base;
    turbo_queue_t* queue = turbo_hashmap_get(engine->queues, (void*)name);

    /*
     * Locks the engine to create a new queue
     */
    pthread_mutex_lock(&engine->lock);

    /*
     * We are going to double check it
     */
    queue = turbo_hashmap_get(engine->queues, (void*)name);
    if(queue == NULL)
    {
        /*
         * Creates a new queue and adds it to the engine
         */
        queue = turbo_queue_create(name);
        turbo_hashmap_put(engine->queues, strdup(name), queue);
    }

    /*
     * Unlock the engine
     */
    pthread_mutex_unlock(&engine->lock);

    return queue;
}
