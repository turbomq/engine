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

#pragma once

#include "types.h"
#include <stdint.h>


#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

typedef uint32_t (*hash_func_t)(void* key);
typedef int (*equals_func_t)(void* a, void* b);
typedef int (*callback_func_t)(void* key, void* value, void* context);

typedef struct  
{    
} turbo_hashmap_t;

/*
 * Creates an instance of a hashmap using hash and equals functions.
 */
turbo_hashmap_t* turbo_hashmap_create(hash_func_t hash, 
                                      equals_func_t equal);

/*
 * Creates an instance of a hashmap for string keys.
 */
turbo_hashmap_t* turbo_hashmap_str_create(void);

/*
 * Creates an instance of a hashmap for integer keys.
 */
turbo_hashmap_t* turbo_hashmap_int_create(void);

/*
 * Returns the size of the given hashmap.
 */
size_t turbo_hashmap_size(turbo_hashmap_t* base);

/*
 * Destroys the given hashmap and frees the resources.
 */
void turbo_hashmap_destroy(turbo_hashmap_t** base, 
                           release_func_t key_release, 
                           release_func_t value_release);

/*
 * Puts a new item in the hashmap.
 */
void* turbo_hashmap_put(turbo_hashmap_t* base, void* key, void* value);

/*
 * Gets the value by the given key. Returns NULL if there is not a saved value by the given key.
 */
void* turbo_hashmap_get(turbo_hashmap_t* base, void* key);

/*
 * Returns zero if the hashmap does not contains the given key.
 */
int turbo_hashmap_contains(turbo_hashmap_t* base, void* key);

/*
 * Returns the value according to the given key and removes the key from hashmap.
 */ 
void* turbo_hashmap_pop(turbo_hashmap_t* base, void* key);

/*
 * Makes a loop on all the saved keys and values in the hashmap.
 */  
void turbo_hashmap_foreach(turbo_hashmap_t* base, 
                           callback_func_t callback,
                           void* context);