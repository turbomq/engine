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
#include "linked_list.h"

typedef struct _turbo_linked_list_node_t {
    void* data;
    struct _turbo_linked_list_node_t* next;
    struct _turbo_linked_list_node_t* prev;
} turbo_linked_list_node_t;

typedef struct 
{
    turbo_linked_list_t parent;
    release_func_t release;
    size_t size;
    turbo_linked_list_node_t* begin;
    turbo_linked_list_node_t* end;
    turbo_linked_list_on_insert_func_t on_insert;
    void* insert_context;
    turbo_linked_list_on_remove_func_t on_remove;
    void* remove_context;
} turbo_linked_list_ex_t;

/*
 * Creates an instance of a linked list.
 */
turbo_linked_list_t* turbo_linked_list_create_ex(release_func_t release, 
                                     turbo_linked_list_on_insert_func_t on_insert,
                                     void* insert_context,
                                     turbo_linked_list_on_remove_func_t on_remove,
                                     void* remove_context)
{
    turbo_linked_list_ex_t* list = (turbo_linked_list_ex_t*)malloc(sizeof(turbo_linked_list_ex_t));
    list->size = 0;
    list->release = release;
    list->begin = NULL;
    list->end = NULL;
    list->on_insert = on_insert;
    list->insert_context = insert_context;
    list->on_remove = on_remove;
    list->remove_context = remove_context;
    return (turbo_linked_list_t*)list;
}

/*
 * Creates an instance of a linked list.
 */
turbo_linked_list_t* turbo_linked_list_create(release_func_t release)
{
    return turbo_linked_list_create_ex(release, NULL, NULL, NULL, NULL);
}

/*
 * Appends an item to the linked list.
 */
int turbo_linked_list_append(turbo_linked_list_t* list, void* data)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    turbo_linked_list_node_t* node = (turbo_linked_list_node_t*)malloc(sizeof(turbo_linked_list_node_t));
    node->data = data;
    if(ex->end == NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        ex->begin = node;
        ex->end = node;
    }
    else
    {
        node->next = NULL;
        node->prev = ex->end;
        ex->end->next = node;
        ex->end = node;
    }
    ex->size += 1;
    
    if(ex->on_insert != NULL)
    {
        ex->on_insert(list, ex->insert_context, ex->size - 1, data);
    }
    return 0;
}

/*
 * Gets an item from the linked list.
 */
turbo_linked_list_node_t* turbo_linked_list_get_node(turbo_linked_list_t* list, size_t index)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    
    if (ex->size == 0 || index >= ex->size || index < 0)
    {
        return NULL;
    }
    
    if(index == 0)
    {
        return ex->begin;
    }
    if(index == ex->size - 1)
    {
        return ex->end;
    }
    
    register size_t pos = 0;
    register size_t direction = 1;
    register turbo_linked_list_node_t* current = ex->begin;
    if(index > ex->size / 2)
    {
        pos = ex->size - 1;
        direction = -1;
        current = ex->end;
    }
    while (current != NULL)
    {
        if(pos == index)
        {
            return current;
        }
        if(direction > 0)
        {
            current = current->next;
        }
        else
        {
            current = current->prev;
        }
        pos = pos + direction;
    }

    return NULL;
}

/*
 * Gets an item from the linked list.
 */
void* turbo_linked_list_get(turbo_linked_list_t* list, size_t index)
{
    turbo_linked_list_node_t* node;
    
    node = turbo_linked_list_get_node(list, index);
    
    if(node != NULL)
    {
        return node->data;
    }
    
    return NULL;
}

/*
 * Sets the item at the given position in the list if it is possible.
 */
int turbo_linked_list_set(turbo_linked_list_t* list, size_t index, void* data)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    turbo_linked_list_node_t* node = turbo_linked_list_get_node(list, index);;
    void* old_data = NULL;

    if(node != NULL)
    {
        old_data = node->data;
        node->data = data;
    }
    
    if(ex->release != NULL && old_data != NULL)
    {
        ex->release(old_data);
    }
    
    return node != NULL ? 0 : -1;
}

/*
 * Returns the size of linked list.
 */
size_t turbo_linked_list_size(turbo_linked_list_t* list)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    return ex->size;
}

/*
 * Removes an item from the linked list.
 */
void turbo_linked_list_remove(turbo_linked_list_t* list, size_t pos)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    void* data = turbo_linked_list_pop(list, pos);
    if(ex->release != NULL)
    {
        ex->release(data);
    }
}

/*
 * Pops an item from the linked list and returns it.
 */
void* turbo_linked_list_pop(turbo_linked_list_t* list, size_t pos)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    turbo_linked_list_node_t* node = turbo_linked_list_get_node(list, pos);;
    void* data = NULL;
    
    if(node != NULL)
    {
        data = node->data;

        turbo_linked_list_node_t* next = node->next;
        turbo_linked_list_node_t* prev = node->prev;

        if(next != NULL)
        {
            next->prev = prev;
        }
        if(prev != NULL)
        {
            prev->next = next;
        }
        
        if(ex->end == node)
        {
            ex->end = prev;
        }
        if(ex->begin == node)
        {
            ex->begin = next;
        }
        
        ex->size -= 1;
        
        if(ex->size == 0)
        {
            ex->begin = NULL;
            ex->end = NULL;
        }
    }
        
    if(ex->on_remove != NULL && node != NULL)
    {
        ex->on_remove(list, ex->remove_context, pos, data);
    }
    
    if(node != NULL)
    {
        free(node);
    }
    
    return data;
}

/*
 * Removes all items from the linked list.
 */
void turbo_linked_list_clear(turbo_linked_list_t* list)
{
    turbo_linked_list_ex_t* ex = (turbo_linked_list_ex_t*)list;
    while(ex->size > 0)
    {
        turbo_linked_list_remove(list, 0);
    }
}

/*
 * Destroys the given linked list.
 */
void turbo_linked_list_destroy(turbo_linked_list_t** list)
{
    turbo_linked_list_t* ptr = *list;
    if(ptr != NULL)
    {
        turbo_linked_list_clear(ptr);
        free(ptr);
        *list = NULL;
    }
}

/*
void add(turbo_linked_list_t* l, int count)
{
    printf("Thread starting\n");
    int i;
    for(i = 0; i < count; i++)
    {
        turbo_linked_list_append(l, &i);
        if(i > 8)
        {
            turbo_linked_list_remove(l, 0);
        }
    }
}

int main()
{
    int i;
    int count = 1000000;
    int tasks = 4;
    turbo_linked_list_t * list = turbo_linked_list_create(NULL);
#pragma omp parallel
    {
#pragma omp single
        {
            for(i = 0; i < tasks; i++)
#pragma omp task
            {
                add(list, count);
            }
        }
    }
    printf("%ld, %d\n", turbo_linked_list_size(list), turbo_linked_list_size(list) == count * tasks);
    turbo_linked_list_destroy(&list);
    return 0;
}
*/