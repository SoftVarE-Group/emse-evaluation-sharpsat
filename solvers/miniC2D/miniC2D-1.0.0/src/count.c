/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

//cache.c
BOOLEAN lookup_cache(VtreeCV* item, DVtree* vtree, VtreeManager* manager);
void insert_cache(VtreeCV item, DVtree* vtree, VtreeManager* manager);
void drop_vtree_cache_entries(DVtree* vtree, VtreeManager* manager);

//local
void count_dispatcher(c2dWmc* count, Clause** learned_clause, DVtree* vtree, VtreeManager* manager, SatState* sat_state);

/******************************************************************************
 * three counting cases: leaf nodes, decomposition nodes, and Shannon nodes
 *
 * all cases take (c2dWmc* count, Clause** learned_clause) as their first arguments
 *
 * after a case returns:
 * --if *learned_clause==NULL, then *count contains the corresponding model count
 * --if *learned_clause!=NULL, then a clause was learned and counting was aborted
 *   (that is, *count is not meaningful)
 *
 * when a clause is learned during the counting process, the learned clause must
 * be asserted (and all learned clauses it leads to must also be asserted) before
 * counting resumes. for that, we backtrack to the assertion level of the learned 
 * clause
 ******************************************************************************/

/******************************************************************************
 * main (weighted) model counting code
 ******************************************************************************/

c2dWmc count_vtree(VtreeManager* manager, SatState* sat_state) {

  c2dWmc count;
  Clause* learned_clause = NULL;
  DVtree* vtree          = manager->vtree;
  
  if(sat_assert_unit_clauses(sat_state)) { //unit resolution succeeded
    count_dispatcher(&count,&learned_clause,vtree,manager,sat_state);
    if(learned_clause!=NULL) count = 0; //cnf is inconsistent
  }
  else count = 0; //cnf is inconsistent

  sat_undo_assert_unit_clauses(sat_state);
  return count;
}

/******************************************************************************
 * three counting cases: leaf nodes, decomposition nodes, and Shannon nodes
 ******************************************************************************/

/******************************************************************************
 * Case I: leaf vtree (count depends on state of associated variable)
 ******************************************************************************/

c2dWmc var2count(Var* var) {
  Lit* plit = sat_var2pliteral(var);
  Lit* nlit = sat_var2nliteral(var);
  if(sat_is_implied_literal(plit))       return sat_literal_weight(plit);
  else if(sat_is_implied_literal(nlit))  return sat_literal_weight(nlit);
  else return (sat_literal_weight(plit) + sat_literal_weight(nlit));
}

void count_vtree_leaf(c2dWmc* count, Clause** learned_clause, DVtree* vtree) {
  assert(vtree_is_leaf(vtree));
  *count = var2count(vtree->var);
  *learned_clause = NULL;
}

/******************************************************************************
 * Case II: decomposition node (left and right vtrees are independent)
 ******************************************************************************/

void count_vtree_decomposed(c2dWmc* count, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, SatState* sat_state) {
    
  c2dWmc l_count;
  count_dispatcher(&l_count,learned_clause,vtree->left,vtree_manager,sat_state);
  if(*learned_clause!=NULL) {
    drop_vtree_cache_entries(vtree->left,vtree_manager);
    return;
  }
  else if(l_count==0) { //optimization
    *count = 0;
    return;
  }
  
  c2dWmc r_count;
  count_dispatcher(&r_count,learned_clause,vtree->right,vtree_manager,sat_state);
  if(*learned_clause!=NULL) {
    drop_vtree_cache_entries(vtree,vtree_manager);
    return; 
  }

  assert(*learned_clause==NULL);
  *count = l_count*r_count;
}

/******************************************************************************
 * Case III: Shannon node (count based on case analysis)
 ******************************************************************************/

void count_vtree_shannon(c2dWmc* count, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, SatState* sat_state);

static inline
BOOLEAN count_with_literal(c2dWmc* count, Clause** learned_clause, Lit* literal, DVtree* vtree, VtreeManager* vtree_manager, SatState* sat_state) {
  *learned_clause     = sat_decide_literal(literal,sat_state);
  if(*learned_clause==NULL) count_dispatcher(count,learned_clause,vtree->right,vtree_manager,sat_state);
  sat_undo_decide_literal(sat_state);
  if(*learned_clause!=NULL) { //a clause was learned
    if(sat_at_assertion_level(*learned_clause,sat_state)) {
      *learned_clause = sat_assert_clause(*learned_clause,sat_state);
      //if another clause was learned, its assertion level must be lower (hence, we must backrack)
      //if another clause was not learned, then we are ready to try vtree again (with the learned clause)
      if(*learned_clause==NULL) count_vtree_shannon(count,learned_clause,vtree,vtree_manager,sat_state);
    }
    return 0; //counting with literal failed as it led to learning at least one clause
  }
  else return 1; //counting with literal succeeded without learning clauses
}

void count_vtree_shannon(c2dWmc* count, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, SatState* sat_state) {
  Var* var = vtree_shannon_var(vtree);
 
  if(sat_is_instantiated_var(var) || sat_is_irrelevant_var(var)) {
    count_dispatcher(count,learned_clause,vtree->right,vtree_manager,sat_state);
    if(*learned_clause==NULL) *count *= var2count(var);
    return;
  }

  Lit* plit = sat_var2pliteral(var);
  Lit* nlit = sat_var2nliteral(var);

  if(!count_with_literal(count,learned_clause,plit,vtree,vtree_manager,sat_state)) return;
  assert(*learned_clause==NULL);
  assert(!sat_instantiated_var(var));
  c2dWmc pcount = *count; //save count conditioned on plit

  if(!count_with_literal(count,learned_clause,nlit,vtree,vtree_manager,sat_state)) return;
  assert(*learned_clause==NULL);
  assert(!sat_instantiated_var(var));
  c2dWmc ncount = *count; //save count conditioned on nlit 

  *count = (pcount*sat_literal_weight(plit)) + (ncount*sat_literal_weight(nlit));
}

/******************************************************************************
 * count dispatcher
 ******************************************************************************/

void count_dispatcher(c2dWmc* count, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, SatState* sat_state) {

  //check cache
  VtreeCV item;
  if(lookup_cache(&item,vtree,vtree_manager)) {
    *count = item.count;
    *learned_clause = NULL;
    return;
  }

  //need to count
  if(vtree_is_leaf(vtree)) 
    count_vtree_leaf(count,learned_clause,vtree);
  else if(vtree_is_shannon_node(vtree))
    count_vtree_shannon(count,learned_clause,vtree,vtree_manager,sat_state);
  else
    count_vtree_decomposed(count,learned_clause,vtree,vtree_manager,sat_state);

  //cache if a count is returned
  if(*learned_clause==NULL) { //otherwise, a count has not been returned
    item.count = *count;
    insert_cache(item,vtree,vtree_manager);
  }
}
 
/******************************************************************************
 * end
 ******************************************************************************/
