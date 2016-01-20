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
#include "queue.h"
#include "hashmap.h"

typedef struct 
{
    turbo_queue_t parent;
    char* name;
    turbo_hashmap_t* map;
    pthread_mutex_t lock;
} turbo_queue_ex_t;

/*
 * Creates a instance of an engine.
 */
turbo_queue_t* turbo_queue_create(const char* name)
{
    turbo_queue_ex_t* queue = (turbo_queue_ex_t*)malloc(sizeof(turbo_queue_ex_t));
    queue->name = strdup(name);
    queue->map = turbo_hashmap_str_create();
    pthread_mutex_init(&queue->lock, NULL);
    return (turbo_queue_t*)queue;
}

void release_fifo(void* fifo)
{
    turbo_fifo_t* ptr = (turbo_fifo_t*)fifo;
    turbo_fifo_destroy(&ptr);
}

/*
 * Destroys the given queue.
 */
void turbo_queue_destroy(turbo_queue_t** base)
{
    turbo_queue_ex_t** queue = (turbo_queue_ex_t**)base;
    turbo_queue_ex_t* ptr = *queue;
    if(ptr != NULL)
    {
        turbo_hashmap_destroy(&ptr->map, free, release_fifo);
        pthread_mutex_destroy(&ptr->lock);
        free(ptr->name);
        free(ptr);
        *queue = NULL;
    }
}

void release_message(void* data)
{
    turbo_message_t* message = (turbo_message_t*)data;
    turbo_message_destroy(&message);
}

turbo_fifo_t* turbo_queue_get_fifo(turbo_queue_ex_t* queue, char* topic)
{
    turbo_fifo_t* fifo = turbo_hashmap_get(queue->map, topic);
    if(fifo == NULL)
    {
        pthread_mutex_lock(&queue->lock);
        fifo = turbo_hashmap_get(queue->map, topic);
        if(fifo == NULL)
        {
            fifo = turbo_fifo_create(release_message);
            if(fifo != NULL)
            {
                turbo_hashmap_put(queue->map, strdup(topic), fifo);
            }
        }
        pthread_mutex_unlock(&queue->lock);
    }
    return fifo;
}

/*
 * Pushs a new message using the topic.
 */
int turbo_queue_push(turbo_queue_t* base, const char* topic, turbo_message_t* message)
{
    turbo_fifo_t* fifo = turbo_queue_get_fifo((turbo_queue_ex_t*)base, (char*)topic);
    if(fifo != NULL)
    {
        turbo_fifo_push(fifo, message);
        return 0;
    }
    return -1;
}

/*
 * Pops a new message using the topic. If there is not a message it is going to wait.
 * However, if in the timeout duration there was noting to push it returns NULL.
 */
turbo_message_t* turbo_queue_pop(turbo_queue_t* base, const char* topic, int timeout)
{
    turbo_fifo_t* fifo = turbo_queue_get_fifo((turbo_queue_ex_t*)base, (void*)topic);
    if(fifo != NULL)
    {
        return turbo_fifo_pop(fifo, timeout);
    }
    return NULL;
}

/*
 * Removes a topic and returns the fifo contains the messages related to topic.
 * It would return NULL if there was not such a topic.
 */
turbo_fifo_t* turbo_queue_remove_topic(turbo_queue_t* base, const char* topic)
{
    turbo_queue_ex_t* queue = (turbo_queue_ex_t*)base;
    turbo_fifo_t* fifo = turbo_hashmap_get(queue->map, (char*)topic);
    if(fifo != NULL)
    {
        pthread_mutex_lock(&queue->lock);
        fifo = turbo_hashmap_pop(queue->map, (char*)topic);
        pthread_mutex_unlock(&queue->lock);
    }
    return fifo;
}