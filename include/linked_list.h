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

#include "types.h"

typedef struct 
{
} turbo_linked_list_t;

typedef void (*turbo_linked_list_on_insert_func_t)(turbo_linked_list_t* list, void* context, size_t pos, void* data);
typedef void (*turbo_linked_list_on_remove_func_t)(turbo_linked_list_t* list, void* context, size_t pos, void* data);

/*
 * Creates an instance of a linked list.
 */
turbo_linked_list_t* turbo_linked_list_create_ex(release_func_t, 
                                     turbo_linked_list_on_insert_func_t on_insert,
                                     void* insert_context,
                                     turbo_linked_list_on_remove_func_t on_remove,
                                     void* remove_context);

/*
 * Creates an instance of a linked list.
 */
turbo_linked_list_t* turbo_linked_list_create(release_func_t);

                                  /*
 * Appends an item to the linked list.
 */
int turbo_linked_list_append(turbo_linked_list_t* list, void* data);

/*
 * Gets an item from the linked list.
 */
void* turbo_linked_list_get(turbo_linked_list_t* list, size_t index);

/*
 * Sets the item at the given position in the list if it is possible.
 */
int turbo_linked_list_set(turbo_linked_list_t* list, size_t index, void* data);

/*
 * Returns the size of linked list.
 */
size_t turbo_linked_list_size(turbo_linked_list_t* list);

/*
 * Removes an item from the linked list.
 */
void turbo_linked_list_remove(turbo_linked_list_t* list, size_t pos);

/*
 * Pops an item from the linked list and returns it.
 */
void* turbo_linked_list_pop(turbo_linked_list_t* list, size_t pos);

/*
 * Removes all items from the linked list.
 */
void turbo_linked_list_clear(turbo_linked_list_t* list);

/*
 * Destroys the given linked list.
 */
void turbo_linked_list_destroy(turbo_linked_list_t** list);

