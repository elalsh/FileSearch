/*
 * Copyright ©2023 Chris Thachuk.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "CSE333.h"
#include "HashTable.h"
#include "HashTable_priv.h"

///////////////////////////////////////////////////////////////////////////////
// Internal helper functions.
//
#define INVALID_IDX -1

// Grows the hashtable (ie, increase the number of buckets) if its load
// factor has become too high.
static void MaybeResize(HashTable *ht);



// Finds and optionally deleted an element in the given with the given key
// and returns true if present
//
// Arguemnts:
// - list: The list to looke the key value pair
// - delete: Option to delete the key value pair from list if found
// - keyvalue: if the key is present, a copy of the (key,value) is
//   returned to the caller via this return parameter.  Note that the
//   (key,value) is left in the HashTable if delete is false, so it is not safe
// for thr   caller to free keyvalue->value.
// - key: the key in the key value pair we are looking for
//
// Returns:
// - false: if the elements with the given key was not found in given list
// - true: if the elements with the given key was found in the given list
static bool findAndDel(LinkedList *list, bool delete, HTKeyValue_t *keyvalue,
        HTKey_t key);


// Finds the next valid bucket index for the HashTable iterator
//
// Arguemnts:
// - iter: the hash table iterator to look for the nest valid bucket for the
//         hash table
//
// ReturnS:
//
// - INVALID_IDXL: if no valid bucket exists, end of table
// - Positive Integer: the next valid index, there is still element left
int getNextIdx(HTIterator *iter);

int HashKeyToBucketNum(HashTable *ht, HTKey_t key) {
  return key % ht->num_buckets;
}

// Deallocation functions that do nothing.  Useful if we want to deallocate
// the structure (eg, the linked list) without deallocating its elements or
// if we know that the structure is empty.
static void LLNoOpFree(LLPayload_t freeme) { }
static void HTNoOpFree(HTValue_t freeme) { }


///////////////////////////////////////////////////////////////////////////////
// HashTable implementation.

HTKey_t FNVHash64(unsigned char *buffer, int len) {
  // This code is adapted from code by Landon Curt Noll
  // and Bonelli Nicola:
  //     http://code.google.com/p/nicola-bonelli-repo/
  static const uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
  static const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
  unsigned char *bp = (unsigned char *) buffer;
  unsigned char *be = bp + len;
  uint64_t hval = FNV1_64_INIT;

  // FNV-1a hash each octet of the buffer.
  while (bp < be) {
    // XOR the bottom with the current octet.
    hval ^= (uint64_t) * bp++;
    // Multiply by the 64 bit FNV magic prime mod 2^64.
    hval *= FNV_64_PRIME;
  }
  return hval;
}

HashTable* HashTable_Allocate(int num_buckets) {
  HashTable *ht;
  int i;

  Verify333(num_buckets > 0);

  // Allocate the hash table record.
  ht = (HashTable *) malloc(sizeof(HashTable));
  Verify333(ht != NULL);

  // Initialize the record.
  ht->num_buckets = num_buckets;
  ht->num_elements = 0;
  ht->buckets = (LinkedList **) malloc(num_buckets * sizeof(LinkedList *));
  Verify333(ht->buckets != NULL);
  for (i = 0; i < num_buckets; i++) {
    ht->buckets[i] = LinkedList_Allocate();
  }

  return ht;
}

void HashTable_Free(HashTable *table,
                    ValueFreeFnPtr value_free_function) {
  int i;

  Verify333(table != NULL);

  // Free each bucket's chain.
  for (i = 0; i < table->num_buckets; i++) {
    LinkedList *bucket = table->buckets[i];
    HTKeyValue_t *kv;

    // Pop elements off the chain list one at a time.  We can't do a single
    // call to LinkedList_Free since we need to use the passed-in
    // value_free_function -- which takes a HTValue_t, not an LLPayload_t -- to
    // free the caller's memory.
    while (LinkedList_NumElements(bucket) > 0) {
      Verify333(LinkedList_Pop(bucket, (LLPayload_t *)&kv));
      value_free_function(kv->value);
      free(kv);
    }
    // The chain is empty, so we can pass in the
    // null free function to LinkedList_Free.
    LinkedList_Free(bucket, LLNoOpFree);
  }

  // Free the bucket array within the table, then free the table record itself.
  free(table->buckets);
  free(table);
}

int HashTable_NumElements(HashTable *table) {
  Verify333(table != NULL);
  return table->num_elements;
}

bool HashTable_Insert(HashTable *table,
                      HTKeyValue_t newkeyvalue,
                      HTKeyValue_t *oldkeyvalue) {
  int bucket;
  LinkedList *chain;

  Verify333(table != NULL);
  MaybeResize(table);

  // Calculate which bucket and chain we're inserting into.
  bucket = HashKeyToBucketNum(table, newkeyvalue.key);
  chain = table->buckets[bucket];

  // STEP 1: finish the implementation of InsertHashTable.
  // This is a fairly complex task, so you might decide you want
  // to define/implement a helper function that helps you find
  // and optionally remove a key within a chain, rather than putting
  // all that logic inside here.  You might also find that your helper
  // can be reused in steps 2 and 3.
  bool ans = findAndDel(chain, true, oldkeyvalue, newkeyvalue.key);
  HTKeyValue_t *pair = (HTKeyValue_t *) malloc(sizeof(HTKeyValue_t));
  Verify333(pair != NULL);
  pair->key = newkeyvalue.key;
  pair->value = newkeyvalue.value;
  LinkedList_Push(chain, (LLPayload_t)pair);
  if (!ans) {
    table->num_elements++;
  }
  return ans;  // you may need to change this return value
}


bool findAndDel(LinkedList *list, bool delete, HTKeyValue_t *keyvalue,
HTKey_t key) {
  // iterate the list and if found return true and optionally delete
  LLIterator *itr = LLIterator_Allocate(list);
  HTKeyValue_t *keyVal;
  while (LLIterator_IsValid(itr)) {
    LLIterator_Get(itr, (LLPayload_t *)&keyVal);
    if (keyVal->key == key) {
      *keyvalue = *keyVal;
      if (delete) {
        LLIterator_Remove(itr, HTNoOpFree);  // remove the key
        free(keyVal);
      }

      // *keyvalue = *keyVal;
      LLIterator_Free(itr);
      return true;  // key was found return true
    }
    LLIterator_Next(itr);
  }
  LLIterator_Free(itr);
  return false;  // the key was not found
}

bool HashTable_Find(HashTable *table,
                    HTKey_t key,
                    HTKeyValue_t *keyvalue) {
  Verify333(table != NULL);

  // STEP 2: implement HashTable_Find.
  // Calculate the bucket index of the key
  int bucketIndex = HashKeyToBucketNum(table, key);
  LinkedList *list = table->buckets[bucketIndex];
  bool ans = findAndDel(list, false, keyvalue, key);
  return ans;  // we return answer of the findAndDel function
}

bool HashTable_Remove(HashTable *table,
                      HTKey_t key,
                      HTKeyValue_t *keyvalue) {
  Verify333(table != NULL);

  // STEP 3: implement HashTable_Remove.
  int bucket = HashKeyToBucketNum(table, key);
  LinkedList *list = table->buckets[bucket];
  bool ans = findAndDel(list, true, keyvalue, key);
  if (ans) {
    table->num_elements--;
  }
  return ans;
}


///////////////////////////////////////////////////////////////////////////////
// HTIterator implementation.

HTIterator* HTIterator_Allocate(HashTable *table) {
  HTIterator *iter;
  int         i;

  Verify333(table != NULL);

  iter = (HTIterator *) malloc(sizeof(HTIterator));
  Verify333(iter != NULL);

  // If the hash table is empty, the iterator is immediately invalid,
  // since it can't point to anything.
  if (table->num_elements == 0) {
    iter->ht = table;
    iter->bucket_it = NULL;
    iter->bucket_idx = INVALID_IDX;
    return iter;
  }

  // Initialize the iterator.  There is at least one element in the
  // table, so find the first element and point the iterator at it.
  iter->ht = table;
  for (i = 0; i < table->num_buckets; i++) {
    if (LinkedList_NumElements(table->buckets[i]) > 0) {
      iter->bucket_idx = i;
      break;
    }
  }
  Verify333(i < table->num_buckets);  // make sure we found it.
  iter->bucket_it = LLIterator_Allocate(table->buckets[iter->bucket_idx]);
  return iter;
}

void HTIterator_Free(HTIterator *iter) {
  Verify333(iter != NULL);
  if (iter->bucket_it != NULL) {
    LLIterator_Free(iter->bucket_it);
    iter->bucket_it = NULL;
  }
  free(iter);
}

// a helper function to check if the if there is a bucket with elements
// past curr itr_bucket_idx
int getNextIdx(HTIterator *iter) {
  if (iter->bucket_idx == INVALID_IDX) {
    return INVALID_IDX;
  }
  int nextValidIdx = INVALID_IDX;
  for (int i = iter->bucket_idx+1; i < iter->ht->num_buckets; i++) {
    if (LinkedList_NumElements(iter->ht->buckets[i]) > 0) {
      nextValidIdx = i;
      break;
    }
  }
  Verify333(nextValidIdx < iter->ht->num_buckets);  // should be < num elements
  return nextValidIdx;
}

bool HTIterator_IsValid(HTIterator *iter) {
  Verify333(iter != NULL);

  // STEP 4: implement HTIterator_IsValid.
  // check if we are in INvalid_indx
  if (iter->bucket_it == NULL) {
    return false;
  }
  if (iter->bucket_idx == INVALID_IDX) {
    return false;
  }
  Verify333(iter->bucket_it != NULL);

  // return true if the current bucket or next valid bucket exists;
  return true;
}

bool HTIterator_Next(HTIterator *iter) {
  Verify333(iter != NULL);

  // STEP 5: implement HTIterator_Next.
  // if is currently not valid return false;
  if (!HTIterator_IsValid(iter)) {
    return false;
  }
  Verify333(iter->bucket_it != NULL);
  LLIterator_Next(iter->bucket_it);  // call next on current bucket iterator


  // if the current buckt is valid, use current bucket
  if (!LLIterator_IsValid(iter->bucket_it)) {  // if the curr bucket is notvalid
    // Check if next valid bucket exitsts and set iter field accordingly
    int nextBucket = getNextIdx(iter);
    if (nextBucket == INVALID_IDX) {  // no next valid bucket exits(end of list)
      iter->bucket_idx = INVALID_IDX;
      LLIterator_Free(iter->bucket_it);
      iter->bucket_it = NULL;
      return false;
    } else {  // a valid bucket exists
      LLIterator_Free(iter->bucket_it);
      iter->bucket_it = NULL;
      iter->bucket_idx = nextBucket;
      iter->bucket_it = LLIterator_Allocate(iter->ht->buckets[iter->bucket_idx]);
      Verify333(iter->bucket_it != NULL);
    }
  }
  return true;
}

bool HTIterator_Get(HTIterator *iter, HTKeyValue_t *keyvalue) {
  Verify333(iter != NULL);

  // STEP 6: implement HTIterator_Get.
  if (iter->ht->num_elements == 0 || !HTIterator_IsValid(iter)) {
    return false;
  }
  HTKeyValue_t *keyVal;
  Verify333(iter->bucket_it != NULL);
  LLIterator_Get(iter->bucket_it, (LLPayload_t *)&keyVal);
  *keyvalue = *keyVal;
  return true;
}

bool HTIterator_Remove(HTIterator *iter, HTKeyValue_t *keyvalue) {
  HTKeyValue_t kv;

  Verify333(iter != NULL);

  // Try to get what the iterator is pointing to.
  if (!HTIterator_Get(iter, &kv)) {
    return false;
  }

  // Advance the iterator.  Thanks to the above call to
  // HTIterator_Get, we know that this iterator is valid (though it
  // may not be valid after this call to HTIterator_Next).
  HTIterator_Next(iter);

  // Lastly, remove the element.  Again, we know this call will succeed
  // due to the successful HTIterator_Get above.
  Verify333(HashTable_Remove(iter->ht, kv.key, keyvalue));
  Verify333(kv.key == keyvalue->key);
  Verify333(kv.value == keyvalue->value);

  return true;
}

static void MaybeResize(HashTable *ht) {
  HashTable *newht;
  HashTable tmp;
  HTIterator *it;

  // Resize if the load factor is > 3.
  if (ht->num_elements < 3 * ht->num_buckets)
    return;

  // This is the resize case.  Allocate a new hashtable,
  // iterate over the old hashtable, do the surgery on
  // the old hashtable record and free up the new hashtable
  // record.
  newht = HashTable_Allocate(ht->num_buckets * 9);

  // Loop through the old ht copying its elements over into the new one.
  for (it = HTIterator_Allocate(ht);
       HTIterator_IsValid(it);
       HTIterator_Next(it)) {
    HTKeyValue_t item, unused;

    Verify333(HTIterator_Get(it, &item));
    HashTable_Insert(newht, item, &unused);
  }

  // Swap the new table onto the old, then free the old table (tricky!).  We
  // use the "no-op free" because we don't actually want to free the elements;
  // they're owned by the new table.
  tmp = *ht;
  *ht = *newht;
  *newht = tmp;

  // Done!  Clean up our iterator and temporary table.
  HTIterator_Free(it);
  HashTable_Free(newht, &HTNoOpFree);
}
