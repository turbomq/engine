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
#include <string.h>
#include "array_list.h"

typedef struct
{
    turbo_array_list_t parent;
    size_t size;
    size_t capacity;
    void** items;
    release_func_t release;
} turbo_array_list_ex_t;

/*
 * Creates an instance of an array list.
 */
turbo_array_list_t* turbo_array_list_create(release_func_t release)
{
    turbo_array_list_ex_t* array = (turbo_array_list_ex_t*)malloc(sizeof(turbo_array_list_ex_t));
    array->size = 0;
    array->release = release;
    array->capacity = 0;
    array->items = NULL;//(void**)malloc(sizeof(void*) * array->capacity);
    return (turbo_array_list_t*)array;
}

/*
 * Provides the required capacity for the given array
 */
int ensure_capacity(turbo_array_list_ex_t* ex)
{
    size_t grow_size;
    if(ex->capacity == 0)
    {
        grow_size = ex->size == 0 ? 8 : ex->size;
        ex->items = (void**)realloc(ex->items, (ex->size + grow_size) * sizeof(void*));
        if(ex->items == NULL)
        {
            return -1;
        }
        ex->capacity = grow_size;
        return 0;
    }
    return 0;
}

/*
 * Appends an item to the array list.
 */
int turbo_array_list_append(turbo_array_list_t* list, void* data)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    if(ensure_capacity(ex) == 0)
    {
        ex->items[ex->size] = data;
        ex->capacity -= 1;
        ex->size += 1;
        return ex->size;
    }
    return -1;
}

/*
 * Gets an item from the array list.
 */
void* turbo_array_list_get(turbo_array_list_t* list, size_t index)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    if(index < 0 || index >= ex->size)
    {
        return NULL;
    }
    return ex->items[index];
}

/*
 * Sets an item in specific place of the array list.
 */
int turbo_array_list_set(turbo_array_list_t* list, size_t index, void* data)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    if(index < 0 || index >= ex->size)
    {
        return -1;
    }
    ex->items[index] = data;
    return 0;
}


/*
 * Returns the size of array list.
 */
size_t turbo_array_list_size(turbo_array_list_t* list)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    return ex->size;
}

/*
 * Removes an item from the array list.
 */
int turbo_array_list_remove(turbo_array_list_t* list, size_t index)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    void* data = turbo_array_list_pop(list, index);
    if(data == NULL)
    {
        return -1;
    }
    if(ex->release != NULL && data != NULL)
    {
        ex->release(data);
    }
    return 0;
}

/*
 * Pops an item from the array list and returns it.
 */
void* turbo_array_list_pop(turbo_array_list_t* list, size_t index)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)list;
    if(index < 0 || index >= ex->size)
    {
        return NULL;
    }

    void* data = ex->items[index];

    if(index < ex->size - 1)
    {
        memcpy(&ex->items[index],
            ex->items[index + 1],
            (ex->size - index - 1) * sizeof(void*));
    }

    ex->size -= 1;
    ex->capacity += 1;

    /*
    if(ex->capacity > ex->size)
    {
        size_t grow_size = 2 * ex->size / 3;
        ex->items = (void**)realloc(ex->items, ex->size + grow_size);
        ex->capacity = grow_size;
    }
    */

    return data;
}

/*
 * Removes all items from the array list.
 */
void turbo_array_list_clear(turbo_array_list_t* list)
{
    while(turbo_array_list_size(list) > 0)
    {
        turbo_array_list_remove(list, turbo_array_list_size(list) - 1);
    }
}

/*
 * Destroys the given array list.
 */
void turbo_array_list_destroy(turbo_array_list_t** list)
{
    turbo_array_list_ex_t* ex = (turbo_array_list_ex_t*)*list;
    if(ex != NULL)
    {
        turbo_array_list_clear(*list);
        free(ex);
        ex = NULL;
    }
}
