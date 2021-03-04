/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

/******************************************************************************
 * component caching is based on the following concepts:
 *
 * --each vtree node is associated with a set of clauses (cnf)
 * --the state of this cnf depends on the current sat state (i.e., the state
 *   of variables set (by decisions) or implied (by unit resolution))
 * --the state of this cnf is identified by a key, which is a bit vector
 * --a cache entry contains a key (cnf) and a cached value (model count, or nnf node)
 * --a key has a hash code, which indexes its cache entry into the cache (array 
 *   of collision lists) 
 *
 * keys and their hash codes are computed dynamically each time a vtree node is
 * visited during model counting or compilation.
 *
 * the space for keys (bit vectors) is allocated before counting/compilation starts
 ******************************************************************************/
 
/******************************************************************************
 * hashcode
 ******************************************************************************/
 
//computes and stores a hash code for the current key associated with vtree
void set_vtree_hashcode(DVtree* vtree) {
  c2dSize size = vtree->key_size;
  BYTE* key    = vtree->key;
  
  HASHCODE hashcode = vtree->position; //was 0
  while(size--) hashcode = 31*hashcode + *key++;
  vtree->key_hashcode = hashcode;
}

/******************************************************************************
 * constructing keys
 ******************************************************************************/

#define SET_NEXT_BIT(bit) {\
  if(bit_count==8) { /* current cell is full */\
    ++cell; /* move to next cell */\
    *cell = 0; /* clear its 8 bits */\
    bit_count = 0; /* no bit has been set in this cell */\
  }\
  *cell <<= 1; /* shift left one bit to make room for new bit */\
  if(bit) *cell |= (BYTE)1; /* set bit if 1, otherwise already 0 */\
  ++bit_count;\
}

//constructs and stores a key for the current cnf associated with a vtree node
//the key is a bit vector, with one bit for each clause (subsumed or not) and 
//two bits for each variable (free, true, false)
void construct_vtree_key(DVtree* vtree) {
  assert(vtree->cached_size!=0);
  
  //last cell may be partially filled
  //initializing cells to 0 ensures that padded bits are always 0
  BYTE* cell      = vtree->key; //first cell to be filled
  *cell           = 0; //clear bits of cell
  short bit_count = 0; //next bit to be set in cell
  
  //iterate over context clauses
  for(c2dSize i=0; i<vtree->contextC->size; i++) {
    Clause* clause = vtree->contextC->set[i]; 
    BOOLEAN bit = sat_is_subsumed_clause(clause);
    SET_NEXT_BIT(bit);
  }
  
  //bits of literals for context clauses
  for(c2dSize i=0; i<vtree->context_in_vars->size; i++) {
    Var* var = vtree->context_in_vars->set[i];
    //00: var is free
    //01: var is false
    //10: var is true
    BOOLEAN pbit = 0;
    BOOLEAN nbit = 0;
    Lit* plit = sat_var2pliteral(var);
    Lit* nlit = sat_var2nliteral(var);
    if(sat_is_implied_literal(plit)) pbit = 1;
    if(sat_is_implied_literal(nlit)) nbit = 1;
    SET_NEXT_BIT(pbit);
    SET_NEXT_BIT(nbit);
  }
 
  set_vtree_hashcode(vtree);
}

/******************************************************************************
 * constructing and freeing space to hold the keys associated with vtree nodes
 ******************************************************************************/

//returns the number of bytes needed to store n bits
static c2dSize bits2bytes(c2dSize n) { 
  c2dSize x = 8*sizeof(BYTE);
  return (n%x? (n/x)+1: n/x);
}

void allocate_vtree_keys(DVtree* vtree, VtreeManager* manager) {
  vtree->key_size    = 0;
  vtree->key         = NULL;
  vtree->cache_entry = NULL;
  
  if(vtree->left!=NULL) {  
    if(vtree->cached_size!=0) {
      c2dSize size    = vtree->cached_size;
      vtree->key_size = bits2bytes(size);
      vtree->key = (BYTE*) calloc(vtree->key_size,sizeof(BYTE));
    }
    allocate_vtree_keys(vtree->left,manager);
    allocate_vtree_keys(vtree->right,manager);
  }
}

void free_vtree_keys(DVtree* vtree) {
  if(vtree->left!=NULL) {
    free(vtree->key);
    free_vtree_keys(vtree->left);
    free_vtree_keys(vtree->right);
  }
}

void allocate_manager_keys(VtreeManager* manager) {
  allocate_vtree_keys(manager->vtree,manager);
}

void free_manager_keys(VtreeManager* manager) {
  free_vtree_keys(manager->vtree);
}

/******************************************************************************
 * end
 ******************************************************************************/
