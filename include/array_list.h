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

typedef struct _turbo_array_list_t
{
} turbo_array_list_t;

/*
 * Creates an instance of an array list.
 */
turbo_array_list_t* turbo_array_list_create(release_func_t relase);

/*
 * Appends an item to the array list.
 */
int turbo_array_list_append(turbo_array_list_t* list, void* data);

/*
 * Gets an item from the array list.
 */
void* turbo_array_list_get(turbo_array_list_t* list, size_t index);

/*
 * Sets an item in specific place of the array list.
 */
int turbo_array_list_set(turbo_array_list_t* list, size_t index, void* data);

/*
 * Returns the size of array list.
 */
size_t turbo_array_list_size(turbo_array_list_t* list);

/*
 * Removes an item from the array list.
 */
int turbo_array_list_remove(turbo_array_list_t* list, size_t pos);

/*
 * Pops an item from the array list and returns it.
 */
void* turbo_array_list_pop(turbo_array_list_t* list, size_t pos);

/*
 * Removes all items from the array list.
 */
void turbo_array_list_clear(turbo_array_list_t* list);

/*
 * Destroys the given array list.
 */
void turbo_array_list_destroy(turbo_array_list_t** list);

