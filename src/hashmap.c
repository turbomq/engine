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
 *    auint32_t with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "hashmap.h"

#define TRUE 1
#define FALSE 0
#define LOAD_FACTOR 4 / 3
#define INITIAL_CAPACITY 16

typedef struct _turbo_hashmap_entry_t 
{
    int hash;
    void* key;
    void* value;
    struct _turbo_hashmap_entry_t* next;
} turbo_hashmap_entry_t;

typedef struct  
{
    turbo_hashmap_t parent;
    turbo_hashmap_entry_t** buckets;
    size_t bucket_count;
    hash_func_t hash;
    equals_func_t equals;
    size_t size;
} turbo_hashmap_ex_t;

turbo_hashmap_t* turbo_hashmap_create(hash_func_t hash, 
                          equals_func_t equals) 
{
    assert(hash != NULL);
    assert(equals != NULL);
    
    turbo_hashmap_ex_t* map = malloc(sizeof(turbo_hashmap_ex_t));
    if (map == NULL) 
    {
        return NULL;
    }
    
    size_t minimum_bucket_count = INITIAL_CAPACITY * LOAD_FACTOR;
    map->bucket_count = 1;
    while (map->bucket_count <= minimum_bucket_count) 
    {
        // Bucket count must be power of 2.
        map->bucket_count <<= 1; 
    }

    map->buckets = calloc(map->bucket_count, sizeof(turbo_hashmap_entry_t*));
    if (map->buckets == NULL) 
    {
        free(map);
        return NULL;
    }
    
    map->size = 0;

    map->hash = hash;
    map->equals = equals;
    
    //mutex_init(&map->lock);
    
    return (turbo_hashmap_t*)map;
}

uint32_t hash_key(turbo_hashmap_t* base, void* key) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    
    uint32_t h = map->hash(key);
    
    /*
     * To avoid bad hashing
     */
    /*
    h += ~(h << 9);
    h ^= (((unsigned uint32_t) h) >> 14);
    h += (h << 4);
    h ^= (((unsigned uint32_t) h) >> 10);
    */ 
    return h;
}

size_t turbo_hashmap_size(turbo_hashmap_t* base) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    return map->size;
}

size_t calculate_index(size_t bucket_count, uint32_t hash) 
{
    return ((size_t) hash) & (bucket_count - 1);
}

void turbo_hashmap_expand(turbo_hashmap_t* base) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    size_t i;
    size_t index;
    turbo_hashmap_entry_t* entry;
    turbo_hashmap_entry_t* next;
    size_t new_bucket_count;
    turbo_hashmap_entry_t** new_buckets;

    if (map->size > (map->bucket_count * 1 / LOAD_FACTOR)) 
    {
        new_bucket_count = map->bucket_count << 1;
        new_buckets = calloc(new_bucket_count, sizeof(turbo_hashmap_entry_t*));
        if (new_buckets == NULL) 
        {
            // Abort expansion.
            return;
        }
        
        for (i = 0; i < map->bucket_count; i++) 
        {
            entry = map->buckets[i];
            while (entry != NULL) 
            {
                next = entry->next;
                index = calculate_index(new_bucket_count, entry->hash);
                entry->next = new_buckets[index];
                new_buckets[index] = entry;
                entry = next;
            }
        }

        free(map->buckets);
        map->buckets = new_buckets;
        map->bucket_count = new_bucket_count;
    }
}

void turbo_hashmap_destroy(turbo_hashmap_t** base, release_func_t key_release, release_func_t value_release) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)(*base);
    if(map != NULL)
    {
        size_t i;
        for (i = 0; i < map->bucket_count; i++) 
        {
            turbo_hashmap_entry_t* entry = map->buckets[i];
            while (entry != NULL) 
            {
                turbo_hashmap_entry_t* next = entry->next;
                if(key_release != NULL && entry->key != NULL)
                {
                    key_release(entry->key);
                }
                if(value_release != NULL && entry->value != NULL)
                {
                    value_release(entry->value);
                }
                free(entry);
                entry = next;
            }
        }
        free(map->buckets);
            free(map);
        map = NULL;
    }
}

int turbo_hashmap_hash(void* key, size_t keySize) 
{
    int h = keySize;
    char* data = (char*) key;
    size_t i;
    for (i = 0; i < keySize; i++) 
    {
        h = h * 31 + *data;
        data++;
    }
    return h;
}

turbo_hashmap_entry_t* turbo_hashmap_entry_create(void* key, uint32_t hash, void* value) 
{
    turbo_hashmap_entry_t* entry = malloc(sizeof(turbo_hashmap_entry_t));
    if (entry == NULL) 
    {
        return NULL;
    }
    entry->key = key;
    entry->hash = hash;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

int turbo_hashmap_equal(void* key_a, 
                  uint32_t hash_a, 
                  void* key_b, 
                  uint32_t hash_b,
                  equals_func_t equals) 
{
    if (key_a == key_b) 
    {
        return TRUE;
    }
    if (hash_a != hash_b) 
    {
        return FALSE;
    }
    return equals(key_a, key_b);
}

void* turbo_hashmap_put(turbo_hashmap_t* base, void* key, void* value) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    uint32_t hash = hash_key(base, key);
    size_t index = calculate_index(map->bucket_count, hash);
    turbo_hashmap_entry_t* current;
    turbo_hashmap_entry_t** p = &(map->buckets[index]);
    
    while (TRUE) 
    {
        current = *p;

        // Add a new entry.
        if (current == NULL) 
        {
            *p = turbo_hashmap_entry_create(key, hash, value);
            if (*p == NULL) 
            {
                errno = ENOMEM;
                return NULL;
            }
            map->size++;
            turbo_hashmap_expand(base);
            return NULL;
        }

        // Replace existing entry.
        if (turbo_hashmap_equal(current->key, current->hash, key, hash, map->equals)) 
        {
            void* oldValue = current->value;
            current->value = value;
            return oldValue;
        }

        // Move to next entry.
        p = &current->next;
    }
}

void* turbo_hashmap_lookup(turbo_hashmap_t* base, void* key, int* found)
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    uint32_t hash = hash_key(base, key);
    size_t index = calculate_index(map->bucket_count, hash);
    turbo_hashmap_entry_t* entry = map->buckets[index];
    
    while (entry != NULL) 
    {
        if (turbo_hashmap_equal(entry->key, entry->hash, key, hash, map->equals)) 
        {
            *found = TRUE;
            return entry->value;
        }
        entry = entry->next;
    }
    
    *found = FALSE;
    return NULL;
}

void* turbo_hashmap_get(turbo_hashmap_t* base, void* key) 
{
    int found;
    return turbo_hashmap_lookup(base, key, &found);
}

int turbo_hashmap_contains(turbo_hashmap_t* base, void* key) 
{
    int found;
    turbo_hashmap_lookup(base, key, &found);
    return found;
}

void* turbo_hashmap_pop(turbo_hashmap_t* base, void* key) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    uint32_t hash = hash_key(base, key);
    size_t index = calculate_index(map->bucket_count, hash);
    // Pointer to the current entry.
    turbo_hashmap_entry_t** p = &(map->buckets[index]);
    turbo_hashmap_entry_t* current;
    
    while ((current = *p) != NULL) 
    {
        if (turbo_hashmap_equal(current->key, current->hash, key, hash, map->equals)) 
        {
            void* value = current->value;
            *p = current->next;
            free(current);
            map->size--;
            return value;
        }
        p = &current->next;
    }

    return NULL;
}

void turbo_hashmap_foreach(turbo_hashmap_t* base, 
                     callback_func_t callback,
                     void* context) 
{
    
    assert(callback != NULL);
    
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;

    size_t i;
    for (i = 0; i < map->bucket_count; i++) 
    {
        turbo_hashmap_entry_t* entry = map->buckets[i];
        while (entry != NULL) 
        {
            turbo_hashmap_entry_t *next = entry->next;
            if (!callback(entry->key, entry->value, context)) 
            {
                return;
            }
            entry = next;
        }
    }
}

size_t turbo_hashmap_capacity(turbo_hashmap_t* base) 
{
    turbo_hashmap_ex_t* map = (turbo_hashmap_ex_t*)base;
    size_t bucket_count = map->bucket_count;
    return bucket_count * 1 / LOAD_FACTOR;
}

uint32_t turbo_hashmap_int_hash(void* key) 
{
    // Return the key value itself.
    uint32_t value = *((int*)key);
    return value;
}

int turbo_hashmap_int_equals(void* key_a, void* key_b) 
{
    int a = *((int*) key_a);
    int b = *((int*) key_b);
    return a == b;
}

turbo_hashmap_t* turbo_hashmap_int_create(void)
{
    return turbo_hashmap_create(turbo_hashmap_int_hash, turbo_hashmap_int_equals);
}

uint32_t str_hash_prime(register char* s)
{
    register uint32_t h = 31 /* also prime */;
    register uint32_t A = 54059;
    register uint32_t B = 76963;
    register uint32_t C = 86969;
    while (*s) {
        h = (h * A) ^ (s[0] * B);
        s++;
    }
    return h % C;
}

uint32_t str_hash_murmur3_32(register char *key, register uint32_t len, uint32_t seed) {
    static const uint32_t c1 = 0xcc9e2d51;
    static const uint32_t c2 = 0x1b873593;
    static const uint32_t r1 = 15;
    static const uint32_t r2 = 13;
    static const uint32_t m = 5;
    static const uint32_t n = 0xe6546b64;
 
    register uint32_t hash = seed;
 
    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *) key;
    int i;
    for (i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
 
        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }
 
    const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
    uint32_t k1 = 0;
 
    switch (len & 3) {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];
 
        k1 *= c1;
        k1 = (k1 << r1) | (k1 >> (32 - r1));
        k1 *= c2;
        hash ^= k1;
    }
 
    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
 
    return hash;
}

uint32_t str_hash_super_fast(register char* data, register uint32_t len) 
{
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

uint32_t turbo_hashmap_str_hash(void* key)
{
    return str_hash_murmur3_32((char*)key, strlen((char*)key), 10);
}

int turbo_hashmap_str_equals(void* key_a, void* key_b) 
{
    int result = strcmp((const char*)key_a, (const char*)key_b);
    return result == 0;
}

turbo_hashmap_t* turbo_hashmap_str_create(void)
{
    return turbo_hashmap_create(turbo_hashmap_str_hash, turbo_hashmap_str_equals);
}

