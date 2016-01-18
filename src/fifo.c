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

#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include "linked_list.h"
#include "fifo.h"

typedef struct 
{
    turbo_fifo_t parent;
    turbo_linked_list_t* list;
    sem_t semaphore;
    pthread_mutex_t lock;
} turbo_fifo_ex_t;

/*
 * Creates a instance of a fifo.
 */
turbo_fifo_t* turbo_fifo_create(release_func_t release_func)
{
    turbo_fifo_ex_t* fifo = (turbo_fifo_ex_t*)malloc(sizeof(turbo_fifo_ex_t));
    sem_init(&fifo->semaphore, 0, 0);
    pthread_mutex_init(&fifo->lock, NULL);
    fifo->list = turbo_linked_list_create(release_func);
    return (turbo_fifo_t*)fifo;
}

/*
 * Destroys the given fifo.
 */
void turbo_fifo_destroy(turbo_fifo_t** fifo)
{
    turbo_fifo_ex_t* ptr = (turbo_fifo_ex_t*)*fifo;
    if(ptr != NULL)
    {
        turbo_linked_list_destroy(&ptr->list);
        sem_destroy(&ptr->semaphore);
        pthread_mutex_destroy(&ptr->lock);
        free(ptr);
        *fifo = NULL;
    }
}

/*
 * Pushs the given data into fifo.
 */
int turbo_fifo_push(turbo_fifo_t* fifo, void* data)
{
    turbo_fifo_ex_t* ptr = (turbo_fifo_ex_t*)fifo;
    pthread_mutex_lock(&ptr->lock);
    int result = turbo_linked_list_append(ptr->list, data);
    pthread_mutex_unlock(&ptr->lock);
    if(result == 0)
    {
        sem_post(&ptr->semaphore);
        return 0;
    }
    return -1;
}

/*
 * Pops one element from fifo. If the timeout exceeded, it would return NULL. 
 * Zero for timeout means wait forever.
 */
void* turbo_fifo_pop(turbo_fifo_t* fifo, int timeout)
{
    turbo_fifo_ex_t* ptr = (turbo_fifo_ex_t*)fifo;
    struct timespec ts;
    if(timeout > 0)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout;
        if(sem_timedwait(&ptr->semaphore, &ts) != 0)
        {
            return NULL;
        }
    }
    else
    {
        if(sem_wait(&ptr->semaphore) != 0)
        {
            return NULL;
        }
    }
    pthread_mutex_lock(&ptr->lock);
    void* result = turbo_linked_list_pop(ptr->list, 0);
    pthread_mutex_unlock(&ptr->lock);
    return result;
}
