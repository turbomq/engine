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

#include "message.h"
#include "fifo.h"

#pragma once

typedef struct
{
} turbo_queue_t;

/*
 * Creates a instance of an engine.
 */
turbo_queue_t* turbo_queue_create(const char* name);

/*
 * Destroys the given queue.
 */
void turbo_queue_destroy(turbo_queue_t** queue);

/*
 * Pushs a new message using the topic.
 */
int turbo_queue_push(turbo_queue_t* queue, const char* topic, turbo_message_t* message);

/*
 * Pops a new message using the topic. If there is not a message it is going to wait.
 * However, if in the timeout duration there was noting to push it returns NULL.
 */
turbo_message_t* turbo_queue_pop(turbo_queue_t* queue, const char* topic, int timeout);

/*
 * Removes a topic and returns the fifo contains the messages related to topic.
 * It would return NULL if there was not such a topic.
 */
turbo_fifo_t* turbo_queue_remove_topic(turbo_queue_t* queue, const char* topic);
