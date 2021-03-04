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
void compile_dispatcher(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state);

/******************************************************************************
 * three compilation cases: leaf nodes, decomposition nodes, and Shannon nodes
 *
 * all cases take (NNF_NODE* node, Clause** learned_clause) as their first arguments
 *
 * after a case returns:
 * --if *learned_clause==NULL, then *node contains the corresponding compilation
 * --if *learned_clause!=NULL, then a clause was learned and compilation was aborted
 *   (that is, *node is not meaningful)
 *
 * when a clause is learned during the compilation process, the learned clause must
 * be asserted (and all learned clauses it leads to must also be asserted) before
 * compilation resumes. for that, we backtrack to the assertion level of the learned 
 * clause
 ******************************************************************************/

/******************************************************************************
 * main compilation code
 ******************************************************************************/
 
//hash table capacity (formula caching and unique nodes)
//primes: 10007, 20011, 30011, 40009, 50021, 65599, 100003, 2015179,  20000003
#define UNIQUE_TABLE_CAPACITY 20000003

extern NNF_NODE ZERO_NNF_NODE;
extern NNF_NODE ONE_NNF_NODE;

NnfManager* compile_vtree(VtreeManager* manager, SatState* sat_state) {

  NNF_NODE node;
  Clause* learned_clause  = NULL;
  DVtree* vtree           = manager->vtree;
  NnfManager* nnf_manager = nnf_manager_new(sat_var_count(sat_state),UNIQUE_TABLE_CAPACITY);

  if(sat_assert_unit_clauses(sat_state)) { //unit resolution succeeded
    compile_dispatcher(&node,&learned_clause,vtree,manager,nnf_manager,sat_state);
    if(learned_clause!=NULL) node = ZERO_NNF_NODE; //cnf is inconsistent
  }
  else node = ZERO_NNF_NODE; //cnf is inconsistent

  sat_undo_assert_unit_clauses(sat_state);
  nnf_manager_set_root(node,nnf_manager);
  return nnf_manager;
}

/******************************************************************************
 * three compilation cases: leaves, decomposition nodes, and Shannon nodes
 ******************************************************************************/

/******************************************************************************
 * Case I: leaf vtree (compilation depends on state of associated variable)
 ******************************************************************************/

NNF_NODE var2nnf(Var* var, NnfManager* nnf_manager) {
  Lit* plit = sat_var2pliteral(var);
  Lit* nlit = sat_var2nliteral(var);
  if(sat_is_implied_literal(plit))      return nnf_literal2node(plit,nnf_manager);
  else if(sat_is_implied_literal(nlit)) return nnf_literal2node(nlit,nnf_manager);
  else return ONE_NNF_NODE;
}

void compile_vtree_leaf(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, NnfManager* nnf_manager) {
  assert(vtree_is_leaf(vtree));
  *node = var2nnf(vtree->var,nnf_manager);
  *learned_clause = NULL;
}

/******************************************************************************
 * Case II: decomposition node (left and right vtrees are independent)
 ******************************************************************************/

void compile_vtree_decomposed(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state) {

  NNF_NODE l_node;
  compile_dispatcher(&l_node,learned_clause,vtree->left,vtree_manager,nnf_manager,sat_state);
  if(*learned_clause!=NULL) {
    drop_vtree_cache_entries(vtree->left,vtree_manager);
    return;
  }

  NNF_NODE r_node;
  compile_dispatcher(&r_node,learned_clause,vtree->right,vtree_manager,nnf_manager,sat_state);
  if(*learned_clause!=NULL) {
    drop_vtree_cache_entries(vtree,vtree_manager);
    return;
  }

  assert(*learned_clause==NULL);
  *node = nnf_conjoin(l_node,r_node,nnf_manager);
}

/******************************************************************************
 * Case III: Shannon node (compilation based on case analysis)
 ******************************************************************************/

void compile_vtree_shannon(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state);

static inline
BOOLEAN compile_with_literal(NNF_NODE* node, Clause** learned_clause, Lit* literal, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state) {
  *learned_clause = sat_decide_literal(literal,sat_state);
  if(*learned_clause==NULL) compile_dispatcher(node,learned_clause,vtree->right,vtree_manager,nnf_manager,sat_state);
  sat_undo_decide_literal(sat_state);
  if(*learned_clause!=NULL) { //a clause was learned
    if(sat_at_assertion_level(*learned_clause,sat_state)) {
      *learned_clause = sat_assert_clause(*learned_clause,sat_state);
      //if another clause was learned, its assertion level must be lower (hence, we must backtrack)
      //if another clause was not learned, then we are ready to try vtree again (with the learned clause)
      if(*learned_clause==NULL) compile_vtree_shannon(node,learned_clause,vtree,vtree_manager,nnf_manager,sat_state);
    }
    return 0; //compiling with literal failed as it led to learning at least one clause
  }
  else return 1; //compiling with literal succeeded without learning clauses
}

void compile_vtree_shannon(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state) {
  Var* var = vtree_shannon_var(vtree);

  if(sat_is_instantiated_var(var) || sat_is_irrelevant_var(var)) {
    compile_dispatcher(node,learned_clause,vtree->right,vtree_manager,nnf_manager,sat_state);
    if(*learned_clause==NULL) *node = nnf_conjoin(*node,var2nnf(var,nnf_manager),nnf_manager);
    return;
  }

  Lit* plit = sat_var2pliteral(var);
  Lit* nlit = sat_var2nliteral(var);

  if(!compile_with_literal(node,learned_clause,plit,vtree,vtree_manager,nnf_manager,sat_state)) return;
  assert(*learned_clause==NULL);
  assert(!sat_instantiated_var(var));
  NNF_NODE pnode = *node; //save the node when conditioned on plit

  if(!compile_with_literal(node,learned_clause,nlit,vtree,vtree_manager,nnf_manager,sat_state)) return;
  assert(*learned_clause==NULL);
  assert(!sat_instantiated_var(var));
  NNF_NODE nnode = *node; //save the node when conditioned on nlit

  if(pnode==nnode) *node = pnode;
  else {
    NNF_NODE pl  = nnf_literal2node(plit,nnf_manager);
    NNF_NODE nl  = nnf_literal2node(nlit,nnf_manager);
    NNF_NODE pc  = nnf_conjoin(pl,pnode,nnf_manager);
    NNF_NODE nc  = nnf_conjoin(nl,nnode,nnf_manager);
    *node        = nnf_disjoin(var,pc,nc,nnf_manager);
  }
}

/******************************************************************************
 * compiler dispatcher
 ******************************************************************************/

void compile_dispatcher(NNF_NODE* node, Clause** learned_clause, DVtree* vtree, VtreeManager* vtree_manager, NnfManager* nnf_manager, SatState* sat_state) {

  //check cache
  VtreeCV item;
  if(lookup_cache(&item,vtree,vtree_manager)) {
    *node = item.node;
    *learned_clause = NULL;
    return;
  }

  //need to compile
  if(vtree_is_leaf(vtree)) 
    compile_vtree_leaf(node,learned_clause,vtree,nnf_manager);
  else if(vtree_is_shannon_node(vtree))
    compile_vtree_shannon(node,learned_clause,vtree,vtree_manager,nnf_manager,sat_state);
  else
    compile_vtree_decomposed(node,learned_clause,vtree,vtree_manager,nnf_manager,sat_state);

  //cache if a node is returned
  if(*learned_clause==NULL) { //otherwise, a node has not been returned
    item.node = *node;
    insert_cache(item,vtree,vtree_manager);
  }
}
 
/******************************************************************************
 * end
 ******************************************************************************/
