/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

//getopt.c
c2dOptions* get_options(int argc, char** argv);
//compile.c
NnfManager* compile_vtree(VtreeManager* manager, SatState* sat_state);
//count.c
c2dWmc count_vtree(VtreeManager* manager, SatState* sat_state);
//cache.c
void print_vtree_cache_stats(VtreeCache* vtree_cache);
//utilities.c
void pprint_bytes(const char* string, c2dSize bytes);
char* extended_file_name(const char* fname, const char* new_extension);
const char* vtree_type(const c2dOptions* options);

/******************************************************************************
 * start
 ******************************************************************************/

int main(int argc, char* argv[]) {

  //get options from command line (and defaults)
  c2dOptions* options = get_options(argc,argv);

  VtreeManager* manager;
  SatState* sat_state;
  clock_t start_t;
  clock_t start_total_t;

  //construct CNF 
  start_total_t = start_t = clock();
  printf("\nConstructing CNF...");
  sat_state = sat_state_new(options->cnf_filename);
  clock_t sat_t = clock()-start_t;
  printf(" DONE");
  printf("\nCNF stats: ");
  printf("\n  Vars=%"PRIvS" / ",sat_var_count(sat_state));
  printf("Clauses=%"PRIvS"",sat_clause_count(sat_state));
  printf("\n  CNF Time\t%0.3fs",((double)(sat_t))/CLOCKS_PER_SEC);

  //construct Vtree
  start_t = clock();
  printf("\nConstructing vtree (from %s)...",vtree_type(options)); fflush(stdout);
  manager = vtree_manager_new(sat_state,options);
  clock_t vtree_t = clock()-start_t;
  printf(" DONE");
  printf("\nVtree stats:");
  printf("\n  "); vtree_print_widths(manager->vtree);
  printf("\n  Vtree Time\t%0.3fs",((double)(vtree_t))/CLOCKS_PER_SEC);
  fflush(stdout);

  if(options->vtree_out_filename!=NULL) {
    printf("\nSaving vtree...");
    vtree_save(options->vtree_out_filename,manager->vtree);
    printf(" DONE");
  }
  if(options->vtree_dot_filename!=NULL) {
    printf("\nSaving vtree (dot)...");
    vtree_save_as_dot(options->vtree_dot_filename,manager->vtree);
    printf(" DONE");
  }

  //(weighted) model counting
  if(options->model_counter) {
    start_t = clock();
    printf("\nCounting..."); fflush(stdout);
    c2dWmc count = count_vtree(manager,sat_state);
    clock_t count_t = clock()-start_t;
    printf(" DONE");
    printf("\n  Learned clauses      \t%"PRIvS"",sat_learned_clause_count(sat_state));
    print_vtree_cache_stats(manager->cache);
    printf("\nCount stats:");
    printf("\n  Count Time\t%0.3fs",((double)(count_t))/CLOCKS_PER_SEC);
    printf("\n  Count \t%0.3"PRIwmcS"",count);
    printf("\nTotal Time: %0.3fs\n\n",((double)clock()-start_total_t)/CLOCKS_PER_SEC);
    free(options);
    vtree_manager_free(manager);
    sat_state_free(sat_state);
    return 0;
  }

  //compile CNF into a Decision-DNNF
  start_t = clock();
  printf("\nCompiling..."); fflush(stdout);
  NnfManager* nnf_manager = compile_vtree(manager,sat_state);
  clock_t comp_t = clock()-start_t;
  printf(" DONE");
  pprint_bytes("\n  NNF memory      \t",nnf_manager_memory(nnf_manager));
  printf("\n  Learned clauses      \t%"PRIvS"",sat_learned_clause_count(sat_state));
  print_vtree_cache_stats(manager->cache);
  printf("\n  Compile Time\t%0.3fs",((double)(comp_t))/CLOCKS_PER_SEC);
	
  char* nnf_fname = extended_file_name(options->cnf_filename,".nnf");

  if(options->in_memory==0) { //save NNF to file
    start_t = clock();
    printf("\nSaving compiled NNF to file...");
    c2dSize n_count, e_count;
    nnf_manager_save_to_file(nnf_fname,nnf_manager,&n_count,&e_count);
    printf(" DONE");
    printf("\n  Save Time       \t%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
    printf("\nNNF stats:");
    printf("\n  Nodes           \t%"PRIvS"",n_count);
    printf("\n  Edges           \t%"PRIvS"",e_count);
    nnf_manager_free(nnf_manager); //manager should be freed as NNF destroyed
  }

  Nnf* nnf = NULL;
  if(options->count_models || options->check_entail) { //further processing is needed
    printf("\nPost compilation");
    if(options->in_memory) { //nnf is in memory
      start_t = clock();
      printf("\n  Extracting NNF...");
      nnf = nnf_manager_extract_nnf(nnf_manager);
      printf(" DONE");
      printf("\n  Extract Time    \t%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
      nnf_manager_free(nnf_manager); //manager should be freed as NNF destroyed
    }
    else { //nnf was already saved to file
      start_t = clock();
      //load nnf from file: different format for nnf
      printf("\n  Loading NNF from file...");
      nnf = nnf_load_from_file(nnf_fname);
      printf(" DONE");
      printf("\n  Load Time       \t%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
    }

    printf("\nNNF stats:");
    printf("\n  Nodes           \t%"PRIvS"",nnf_node_count(nnf));
    printf("\n  Edges           \t%"PRIvS"",nnf_edge_count(nnf));
  }
  else { //done: no further processing
    if(options->in_memory) { 
      c2dSize n_count = 0; c2dSize e_count = 0;
      NNF_NODE root = nnf_manager_get_root(nnf_manager);
      nnf_count_nodes(root,&n_count,&e_count);
      nnf_manager_free(nnf_manager);
      printf("\nNNF stats:");
      printf("\n  Nodes           \t%"PRIvS"",n_count);
      printf("\n  Edges           \t%"PRIvS"",e_count);
    }
    printf("\nTotal Time: %0.3fs\n\n",((double)clock()-start_total_t)/CLOCKS_PER_SEC);
    free(options);
    free(nnf_fname);
    vtree_manager_free(manager);
    sat_state_free(sat_state);
    return 0;
  }
	
  //further processing of the nnf is required
  if(options->count_models) {
    start_t = clock();
    printf("\n  Counting...");
    c2dSize var_count = sat_var_count(sat_state);
    char* str = nnf_count_models(var_count,nnf);
    printf(" %s models / ",str);
    printf("%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
    free(str);
  }
	
  if(options->check_entail) {
    BOOLEAN decomposable = 1;
    start_t = clock();
    printf("\n  Checking decomposability... "); fflush(stdout);
    if(nnf_decomposable(nnf)) printf("OK / ");
    else {
      decomposable = 0;
      printf("Failed!!! / "); 
    }
    printf("%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
    start_t = clock();
    printf("\n  Checking entailment... "); fflush(stdout);
    if(nnf_entails_cnf(nnf,sat_state)) printf("OK / ");
    else if(decomposable==1) printf("Failed!!! / ");
    else printf("Cannot decide!!! / ");
    printf("%0.3fs",((double)clock()-start_t)/CLOCKS_PER_SEC);
  }

  printf("\nTotal Time: %0.3fs\n\n",((double)clock()-start_total_t)/CLOCKS_PER_SEC);

  free(options);
  free(nnf_fname);
  nnf_free(nnf);
  vtree_manager_free(manager);
  sat_state_free(sat_state);
  return 0;
}

/******************************************************************************
 * end
 ******************************************************************************/
