/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#ifndef NNFAPI_H_
#define NNFAPI_H_

/******************************************************************************
 * nnf_api.h shows the function prototypes implemented in libnnf.a.
 *
 * the nnf manager uses the NNF_NODE type for representing nnf nodes. this
 * representation is designed to be more appropriate for the compilation phase.
 *
 * once compilation is finished, one can map the compiled nnf into the Nnf type,
 * which is more suitable for further operations on nnf nodes, such as model
 * counting, clausal entailment, and save/load.
 *
 * the nnf manager allows users to obtain nnf nodes corresponding to cnf literals,
 * and to conjoin and disjoin nnf nodes. it also allows designating one nnf node
 * as the manager's root node, which is used in various operations.
 ******************************************************************************/

/******************************************************************************
 * function prototypes
 ******************************************************************************/

//creates a new nnf manager given a sat state and a hash capacity for unique nodes
NnfManager* nnf_manager_new(c2dSize var_count, c2dSize capacity);

//frees the nnf manager
void nnf_manager_free(NnfManager* manager);

//returns the memory used by the nnf manager as a number of bytes
c2dSize nnf_manager_memory(const NnfManager* manager);

//returns the nnf node corresponding to a literal (which is obtained from a sat state)
NNF_NODE nnf_literal2node(const Lit* lit, const NnfManager* manager);

//returns the result of conjoining two nnf nodes
NNF_NODE nnf_conjoin(NNF_NODE node1, NNF_NODE node2, NnfManager* manager);

//returns the result of disjoining two nnf nodes
//
//node1 and node2 can be in one of the following forms:
//--one of them is ONE_NNF_NODE  (the true function):  the result is ONE_NNF_NODE
//--one of them is ZERO_NNF_NODE (the false function): the result is the other node
//--node1 = x.f and node2 = ~x.g, where x is a literal of var: the result is node1+node2
NNF_NODE nnf_disjoin(const Var* var, NNF_NODE node1, NNF_NODE node2, NnfManager* manager);

//returns the node and edge count of the nnf rooted at node
void nnf_count_nodes(NNF_NODE node, c2dSize* node_count, c2dSize* edge_count);

//sets the manager's root nnf node
void nnf_manager_set_root(NNF_NODE node, NnfManager* manager);

//returns the manager's root nnf node
//assumes that the manager's root has been set
NNF_NODE nnf_manager_get_root(const NnfManager* manager);

//returns the manager's root nnf node as type Nnf (within the manager, nnf nodes are of type NNF_NODE)
//assumes that the manager's root has been set
//the returned nnf can be used for further operations, such as model counting,
//clausal entailment, and save/load (see below)
Nnf* nnf_manager_extract_nnf(NnfManager* manager);

//saves the manager's nnf to file, and returns the node and edge count of the saved nnf
//assumes that the manager's root has been set as this designates the nnf to be saved
void nnf_manager_save_to_file(const char* fname, NnfManager* manager, c2dSize* node_count, c2dSize* edge_count);

//
//operations on nnfs of type Nnf 
//these nnfs are either extracted from an nnf manager, or loaded from file
//

//saves an nnf to file
void nnf_save_to_file(const char* fname, const Nnf* nnf);

//loads an nnf from file
Nnf* nnf_load_from_file(const char* fname);

//saves an nnf as a dot file
void nnf_save_as_dot(const char* fname, const Nnf* nnf);

//frees an nnf
c2dSize nnf_free(Nnf* nnf);

//returns the node count of the nnf
c2dSize nnf_node_count(const Nnf* nnf);

//returns the edge count of the nnf
c2dSize nnf_edge_count(const Nnf* nnf);

//returns the model count of the nnf (over the given number of variables)
//var_count must be no less than the number of variables appearing in the nnf
char* nnf_count_models(c2dSize var_count, const Nnf* nnf);

//returns 1 if the nnf entails the cnf of the given sat state, 0 otherwise
BOOLEAN nnf_entails_cnf(const Nnf* nnf, const SatState* sat_state);

//returns 1 if the nnf is decomposable, 0 otherwise
//this is used for error checking, as the compiled nnf must be decomposable
BOOLEAN nnf_decomposable(const Nnf* nnf);

#endif //NNFAPI_H_

/******************************************************************************
 * end
 ******************************************************************************/
