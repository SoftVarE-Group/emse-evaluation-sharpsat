/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#include "c2d.h"

/******************************************************************************
 * various utilities
 ******************************************************************************/

#define KB 1024
#define MB (1024*1024)
#define GB (1024*1024*1024)

//pretty prints the number of bytes next to a supplied string
void pprint_bytes(const char* string, c2dSize bytes) {
  if(bytes < MB) printf("%s%0.1f KB",string,(double)bytes/KB);
  else if(bytes < GB) printf("%s%0.1f MB",string,(double)bytes/MB);
  else printf("%s%0.1f GB",string,(double)bytes/GB);
}

//augments fname with new_extension
char* extended_file_name(const char* fname, const char* new_extension) {
  unsigned size = strlen(fname); //size of fname excluding . and extension
  //allocate space for new file name
  char* new_fname = (char*) calloc(1+size+strlen(new_extension),sizeof(char));
  strncpy(new_fname,fname,size); //copy old fname
  new_fname[size] = '\0'; //append null character at end
  strcat(new_fname,new_extension); //append new extension
  return new_fname;
}

//returns a string description of the vtree type
const char* vtree_type(const c2dOptions* options) {
  if(options->vtree_in_filename!=NULL) return options->vtree_in_filename;
  else if(options->vtree_type=='p') return "primal graph";
  else return "incidence graph";
}

/******************************************************************************
 * end
 ******************************************************************************/
