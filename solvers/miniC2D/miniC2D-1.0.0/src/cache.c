/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

//key.c
void construct_vtree_key(DVtree* vtree);
//utilities.c
void pprint_bytes(const char* string, c2dSize bytes);

//local declarations
BOOLEAN match_keys(register BYTE* key1, register BYTE* key2, register c2dSize size);
void copy_key(register BYTE* key1, register BYTE* key2, register c2dSize size);

/******************************************************************************
 * the cache is implemented as an array of linked lists:
 *
 * --the elements of each linked list are cache entries
 * --a cache entry contains a key (identifies a cnf) and a computed value (count or nnf node)
 * --each key has a hash code (a number)
 * --cache entries in the same linked list have the same hashcode
 * --the hashcode indexes a cache entry into the cache (array of linked lists)
 *
 * each vtree node has a list of cache entries associated with it (i.e., cache entries
 * for cnfs that are associated with that vtree node). this additional indexing
 * facilitates dropping cache entries that are associated with a given vtree node
 ******************************************************************************/
 
/******************************************************************************
 * constructing and freeing a cache
 *
 * these functions are called when constructing or freeing a vtree manager
 ******************************************************************************/

VtreeCache* construct_vtree_cache(c2dSize capacity) {
  VtreeCache* cache = (VtreeCache*) malloc(sizeof(VtreeCache));
  
  cache->buckets    = (VtreeCE**) calloc(capacity,sizeof(VtreeCE*));
  cache->capacity   = capacity;
  cache->count      = 0;
  cache->memory     = 0;
  cache->hits       = 0;
  cache->misses     = 0;
  return cache;
}

void free_cache_entry(VtreeCE* entry) {
  free(entry->key);
  free(entry);
}

void free_vtree_cache(VtreeCache* cache) {
  //free cache entries
  for(c2dSize i=0; i<cache->capacity; i++) {
    VtreeCE* entry = cache->buckets[i];
    while(entry!=NULL) {
      VtreeCE* next = entry->next;
      free_cache_entry(entry);
      entry = next;
    }
  }
  
  free(cache->buckets); //free hash table
  free(cache);
}

/******************************************************************************
 * which vtree nodes to cache at: CRITICAL to performance
 ******************************************************************************/
 
static BOOLEAN should_cache(const DVtree* vtree) {
  return vtree->live_cache && 
         vtree_is_shannon_node(vtree) && 
         !sat_is_instantiated_var(vtree_shannon_var(vtree));
}

/******************************************************************************
 * lookup
 ******************************************************************************/

//returns 1 if lookup is successful, 0 otherwise
//if lookup is successful, set the value of result accordingly
BOOLEAN lookup_cache(VtreeCV* result, DVtree* vtree, VtreeManager* manager) {
  if(!should_cache(vtree)) return 0;
  assert(vtree->cached_size!=0);
  
  //capture the state of cnf associated with vtree as a bit vector and corresponding hash code
  construct_vtree_key(vtree); 
  //the following fields are now current
  BYTE* key         = vtree->key; //bit vector
  c2dSize size      = vtree->key_size;
  HASHCODE hashcode = vtree->key_hashcode;
    
  VtreeCache* cache = manager->cache;
  c2dSize index     = hashcode % cache->capacity;
  VtreeCE* entry    = cache->buckets[index]; //first entry in collision list
  
  while(entry!=NULL) {
    if(vtree==entry->vtree && match_keys(key,entry->key,size)) {
      //hit
      ++cache->hits;
      *result = entry->value;
      return 1;
    }
    else entry = entry->next;
  }

  //miss
  ++cache->misses;
  
  return 0;
}
 
/******************************************************************************
 * insert
 ******************************************************************************/

//inserts a computed value (count or nnf node) into the cache
//the computed value is associated with the current cnf associated with the vtree node 
//assumes that lookup_cache has been already called to set the cnf key and hashcode
void insert_cache(VtreeCV item, DVtree* vtree, VtreeManager* manager) {  
  if(!should_cache(vtree)) return;
  assert(vtree->cached_size!=0); 
    
  //key and hashcode are assumed current
  VtreeCache* cache   = manager->cache;
  HASHCODE hashcode   = vtree->key_hashcode;
  BYTE* key           = vtree->key;
  c2dSize key_size    = vtree->key_size;
  c2dSize index       = hashcode % cache->capacity;
  VtreeCE* head_entry = cache->buckets[index]; //head of collision list
  
  //create entry
  VtreeCE* entry   = (VtreeCE*) malloc(sizeof(VtreeCE));
  entry->value     = item;
  entry->vtree     = vtree;
  entry->key       = (BYTE*) calloc(key_size,sizeof(BYTE));
  copy_key(key,entry->key,key_size); //entry key  
     
  //insert into hash table
  entry->next           = head_entry;
  cache->buckets[index] = entry; 
  entry->prev_next      = cache->buckets+index;
  if(head_entry!=NULL) head_entry->prev_next = &(entry->next);
  
  //add entry to list of cache entries for vtree
  entry->vtree_next  = vtree->cache_entry;
  vtree->cache_entry = entry;
  
  //update stats
  ++cache->count;
  cache->memory += sizeof(VtreeCE) + sizeof(BYTE)*vtree->key_size;
}
 
/******************************************************************************
 * dropping entries
 ******************************************************************************/

//removes cache entry from cache
void drop_cache_entry(VtreeCE* entry, VtreeCache* cache) {
  //remove from collision list
  *(entry->prev_next) = entry->next; 
  if(entry->next!=NULL) entry->next->prev_next = entry->prev_next;
  //update stats
  --cache->count;
  cache->memory -= sizeof(VtreeCE) + sizeof(BYTE)*entry->vtree->key_size;
  //free
  free_cache_entry(entry);
}

//drops all cache entries of vtree and its descendants
void drop_vtree_cache_entries(DVtree* vtree, VtreeManager* manager) {
  if(vtree->left==NULL) return;
  
  VtreeCache* cache = manager->cache;
  VtreeCE* entry    = vtree->cache_entry;
  
  while(entry!=NULL) {
    VtreeCE* next = entry->vtree_next; //next in vtree list of entries
    drop_cache_entry(entry,cache);
    entry = next;
  }
  vtree->cache_entry = NULL;
  
  drop_vtree_cache_entries(vtree->left,manager);
  drop_vtree_cache_entries(vtree->right,manager);
}
 
/******************************************************************************
 * cache stats
 ******************************************************************************/

void clist_size(VtreeCache* cache, c2dSize* max, double* ave, double* ave_key, double* max_key, double* min_key) {
  *max = 0;
  *ave = 0;
  *ave_key = 0;
  *max_key = 0;
  *min_key = 10000000;

  c2dSize bcount = 0; //number of non-empty buckets
  for(c2dSize i=0; i<cache->capacity; i++) {
    c2dSize count = 0;
    VtreeCE* entry = cache->buckets[i];
    while(entry != NULL) {
      *ave_key += entry->vtree->key_size;
      if(entry->vtree->key_size > *max_key) *max_key = entry->vtree->key_size;
      if(entry->vtree->key_size < *min_key) *min_key = entry->vtree->key_size;
      ++count;
      entry = entry->next;
    }
    if(count) { //non-empty bucket
      ++bcount;
      *ave += count;
    }
    if(count > *max) {
      *max = count;
    }
  }
  *ave = *ave/bcount;
  *ave_key = *ave_key/cache->count;
}

void print_vtree_cache_stats(VtreeCache* cache) {
  c2dSize max_cl;
  double ave_cl;
  double ave_key, max_key, min_key;
  clist_size(cache,&max_cl,&ave_cl,&ave_key,&max_key,&min_key);
  
  printf("\nCache stats:");
  printf(     "\n  hit rate   \t%.1f%%",(100.0*cache->hits)/(cache->hits+cache->misses));
  printf(     "\n  lookups    \t%"PRIvS"",cache->hits+cache->misses);
  printf(     "\n  ent count  \t%"PRIvS"",cache->count);
  pprint_bytes("\n  ent memory \t",cache->memory);
  pprint_bytes("\n  ht  memory \t",cache->capacity*sizeof(VtreeCE*));
  printf(     "\n  clists     \t%0.1f ave, %"PRIvS" max",ave_cl,max_cl);
  printf(     "\n  keys       \t%.1fb ave, %.1fb max, %.1fb min",ave_key,max_key,min_key);
}

/******************************************************************************
 * utilities 
 ******************************************************************************/

BOOLEAN match_keys(register BYTE* key1, register BYTE* key2, register c2dSize count) {
  while(count--) if(*key1++ != *key2++) return 0;
  return 1;
}

void copy_key(register BYTE* key, register BYTE* cells, register c2dSize count) {
  while(count--) *cells++ = *key++;
}

/******************************************************************************
 * end
 ******************************************************************************/
