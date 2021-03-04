/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

//local
void print_error_and_exit(const char* message, const char* PACKAGE, int exit_value);
void print_help(const char* PACKAGE, int exit_value);
const char* c2d_version();

/******************************************************************************
 * miniC2D options (default values)
 ******************************************************************************/

#define VTREE_TYPE     'p';
#define VTREE_METHOD   0;
#define VTREE_COUNT    25;
#define INITIAL_UBFS   25;
#define FINAL_UBFS     25;
#define CACHE_CAPACITY 20000003;

#define IN_MEMORY    0;
#define CHECK_ENTAIL 0;
#define COUNT_MODELS 0;
#define COUNTER      0;

/******************************************************************************
 * miniC2D options 
 ******************************************************************************/

c2dOptions* init_options() {
  c2dOptions* options = (c2dOptions*) malloc(sizeof(c2dOptions));

  options->cnf_filename       = NULL;
  options->vtree_in_filename  = NULL;           
  options->vtree_out_filename = NULL;
  options->vtree_dot_filename = NULL; 
  options->vtree_type         = VTREE_TYPE;
  options->vtree_method       = VTREE_METHOD;
  options->vtree_count        = VTREE_COUNT;
  options->initial_ubfs       = INITIAL_UBFS;
  options->final_ubfs         = FINAL_UBFS;
  options->cache_capacity     = CACHE_CAPACITY;
  options->in_memory          = IN_MEMORY;
  options->check_entail       = CHECK_ENTAIL;
  options->count_models       = COUNT_MODELS;
  options->model_counter      = COUNTER;
  options->help               = 0;
  return options;
}

c2dOptions* get_options(int argc, char** argv) {

  c2dOptions* options = init_options();
  while(1) {
    static struct option long_options[] = {
      {"cnf",            required_argument, 0, 'c'},
      {"vtree",          required_argument, 0, 'v'},
      {"vtree_out",      required_argument, 0, 'o'},
      {"vtree_dot",      required_argument, 0, 'd'},
      {"vtree_type",     required_argument, 0, 't'},
      {"vtree_method",   required_argument, 0, 'm'},
      {"vtree_count",    required_argument, 0, 'b'},
      {"initial_ubfs",   required_argument, 0, 'u'},
      {"final_ubfs",     required_argument, 0, 'f'},
      {"cache_capacity", required_argument, 0, 's'},
      {"in_memory",      no_argument,       0, 'i'},
      {"check_entail",   no_argument,       0, 'E'},
      {"count_models",   no_argument,       0, 'C'},
      {"model_counter",  no_argument,       0, 'W'},
      {"help",           no_argument,       0, 'h'},
      {0,                0,                 0,  0}
    };

    int index = 0;
    int argument = getopt_long(argc,argv,"c:v:o:d:t:m:b:u:f:s:iECWh",long_options,&index);
    if(argument==-1) break;

    switch(argument) {
      case 'c': options->cnf_filename       = optarg;        break;
      case 'v': options->vtree_in_filename  = optarg;        break;
      case 'o': options->vtree_out_filename = optarg;        break;
      case 'd': options->vtree_dot_filename = optarg;        break;
      case 't': options->vtree_type         = optarg[0];     break;
      case 'm': options->vtree_method       = atoi(optarg);  break;
      case 'b': options->vtree_count        = atoi(optarg);  break;
      case 'u': options->initial_ubfs       = atoi(optarg);  break;
      case 'f': options->final_ubfs         = atoi(optarg);  break;
      case 's': options->cache_capacity     = atoi(optarg);  break;
      case 'i': options->in_memory          = 1;             break;
      case 'E': options->check_entail       = 1;             break;
      case 'C': options->count_models       = 1;             break;
      case 'W': options->model_counter      = 1;             break;
      case 'h': options->help               = 1;             break;
      default: print_error_and_exit(NULL,C2D_PACKAGE,1);
    }
  }

  //if help is asked, print it and exit
  if(options->help) print_help(C2D_PACKAGE,0);

  //checking validity of options  
  if(options->cnf_filename==NULL) 
    print_error_and_exit("must specify a CNF file",C2D_PACKAGE,1);
  if(options->vtree_type!='p' && options->vtree_type!='i') 
    print_error_and_exit("option -t must be 'p' or 'i'",C2D_PACKAGE,1);
  if(options->vtree_method < 0 || options->vtree_method > 4)
    print_error_and_exit("option -m must be 0, 1, 2, 3 or 4",C2D_PACKAGE,1);
  if(options->vtree_count < 1)
    print_error_and_exit("option -b must be greater than 0",C2D_PACKAGE,1);
  if(options->initial_ubfs < 1 || options->initial_ubfs > 49)
    print_error_and_exit("option -u must be between 1 and 49 (inclusive)",C2D_PACKAGE,1);
  if(options->final_ubfs < 1 || options->final_ubfs > 49)
    print_error_and_exit("option -f must be between 1 and 49 (inclusive)",C2D_PACKAGE,1);
  if(options->initial_ubfs > options->final_ubfs)
    print_error_and_exit("option -u must be less than or equal to option -f",C2D_PACKAGE,1);
  if(options->cache_capacity < 1)
    print_error_and_exit("option -s must be greater than 0",C2D_PACKAGE,1);
  return options;
}

void print_error_and_exit(const char* message, const char* PACKAGE, int exit_value) {
  if(message!=NULL) fprintf(stderr,"%s: %s\n",PACKAGE,message);
  fprintf(stderr,"%s: use --help for more information\n",PACKAGE);
  exit(exit_value);
}

void print_help(const char* PACKAGE, int exit_value) {
  printf("%s: A CNF to Decision-DNNF Compiler and A Weighted Model Counter\n", PACKAGE);
  printf("%s\n",c2d_version());

  printf("%s [-c .] [-v .] [-o .] [-d .] [-t .] [-m .] [-b .] [-u .] [-f .] [-s .]   [-i] [-E] [-C] [-W] [-h]\n", PACKAGE);

  printf("  --cnf             -c FILE    set input CNF file\n");
 
  printf("  --vtree           -v FILE    set input  VTREE file\n");
  printf("  --vtree_out       -o FILE    set output VTREE file\n");
  printf("  --vtree_dot       -d FILE    set output VTREE (dot) file\n");

  printf("  --vtree_type      -t TYPE    set graph abstraction used in vtree construction\n");  
  printf("                               p: primal graph (default)\n");
  printf("                               i: incidence graph\n");

  printf("  --vtree_method    -m METHOD  specify the method for constructing vtree\n");
  printf("                               0: hypergraph with random balance factor (default)\n");
  printf("                               1: hypergraph with fixed balance factor\n");
  printf("                               2: natural elimination order (1,2,...,n)\n");
  printf("                               3: reverse elimination order\n");
  printf("                               4: minfill elimination order\n");
    
  printf("  --vtree_count     -b COUNT   set the number of vtrees to be generated with options -m 0 and -m 1 (default 25)\n");

  printf("  --initial_ubfs    -u FACTOR  set start balance factor when using option -m 1 (default 25, must be between 1 and 49, inclusive)\n");
  printf("  --final_ubfs      -f FACTOR  set end balance factor when using   option -m 1 (default 25, must be between 1 and 49, inclusive)\n");

  printf("  --cache_capacity  -s SIZE    set the hash table capacity for the vtree\n");

  printf("  --in_memory       -i         suppress the saving of compiled NNF to a file\n");
  printf("  --check_entail    -E         verify the compiled Decision-DNNF is correct by ensuring it is decomposable and also entails the input CNF\n");
  printf("  --count_models    -C         count the models of the input CNF after compiling it into a Decision-DNNF\n");
  printf("  --model_counter   -W         count the (weighted) models of the input CNF without compiling it into a Decision-DNNF\n");
  printf("  --help            -h         print this help and exit\n");
  exit(exit_value);
}

const char* c2d_version() {
  return C2D_PACKAGE " version " C2D_VERSION ", " C2D_DATE "\n";
}

/******************************************************************************
 * end
 ******************************************************************************/
