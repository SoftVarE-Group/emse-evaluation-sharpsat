/******************************************************************************
 * The miniC2D Package
 * miniC2D version 1.0.0, Sep 27, 2015
 * http://reasoning.cs.ucla.edu/minic2d
 ******************************************************************************/

#ifndef SATAPI_H_
#define SATAPI_H_

/******************************************************************************
 * the sat library (libsat.a) embeds the sat-solving technology needed to
 * support miniC2D.
 *
 * sat_api.h shows the function prototypes implemented by the sat library.
 *
 * to base miniC2D on some sat solver, simply provide a corresponding sat library
 * that implements the internface described below.
 *
 * the sat library is based on the notion of a "sat state", which is constructed
 * from an input cnf file.
 *
 * the input file is a standard DIMACS, except that a comment line can specify literal
 * weights (needed for weighted model counting). if we have n variables, then weights
 * can be specified using the following comment line:
 *
 *  c weights PW_1 NW_1 ... PW_n NW_n
 *
 * here, PW_i is the weight of pos literal i, and NW_i is the weight of neg literal -i.
 * if no such comment line is given, weights are assumed to be all 1.
 *
 * clauses appearing in the input cnf file are called "input clauses", in contrast
 * to "learned clauses."
 *
 * in addition to a sat state, the sat library assumes structures for the following
 * entities: variables, literals, and clauses.
 *
 * the sat interface corresponds to four sets of functions, one set for each of the
 * four structures: variables, literals, clauses, and sat state).
 *
 * each variable has an index i, 1 <= i <= n, where n is the number of cnf variables.
 *
 * each variable has two literals, one positive and one negative. if a variable has
 * index i, its positive literal has index i, and its negative literal has index -i.
 *
 * each input clause has an index i, 1 <= i <= m, where m is the number of input
 * cnf clauses.
 *
 * a sat state has a "decision level", which is incremented when a literal is
 * decided, and decremented when a literal is undecided. hence, each decided
 * literal is associated with a decision level. similarly, literals which are
 * implied by unit resolution are also associated with a decision level.
 *
 * a learned clause is associated with an "assertion level". the clause can be
 * asserted (added) to a state state only when the decision level of the sat
 * state equals to the clause's assertion level.
 ******************************************************************************/

/******************************************************************************
 * function prototypes (assuming the following typedefs):
 *
 * typedef char BOOLEAN;
 * typedef unsigned long c2dSize;
 * typedef signed long c2dLiteral;
 * typedef double c2dWmc;
 *
 * typedef struct var_t Var;
 * typedef struct literal_t Lit;
 * typedef struct clause_t Clause;
 * typedef struct sat_state_t SatState;
 ******************************************************************************/

/******************************************************************************
 * variables
 ******************************************************************************/

//returns the variable with the given index
Var* sat_index2var(c2dSize index, const SatState* sat_state);

//returns the index of a variable
c2dSize sat_var2index(const Var* var);

//returns the variable of a literal
Var* sat_literal2var(const Lit* lit);

//returns 1 if the variable is instantiated, 0 otherwise
//a variable is instantiated either by decision or implication (by unit resolution)
BOOLEAN sat_is_instantiated_var(const Var* var);

//returns 1 if all input-clauses mentioning the variable are subsumed, 0 otherwise
BOOLEAN sat_is_irrelevant_var(const Var* var);

//returns the number of variables in the cnf
c2dSize sat_var_count(const SatState* sat_state);

//returns the number of input-clauses mentioning a variable
//a variable is mentioned by a clause if one of its literals appears in the clause
c2dSize sat_var_occurences(const Var* var);

//returns the i^th occurence (input-clause) of a variable, 1 <= i < k, where k is
//the number of variable occurrences (clauses are ordered by their indices)
Clause* sat_clause_of_var(c2dSize i, const Var* var);

//returns 1 if a variable is marked, 0 otherwise
//all variables are initially unmarked
BOOLEAN sat_is_marked_var(const Var* var);

//marks a variable
void sat_mark_var(Var* var);

//unmarks a variable
void sat_unmark_var(Var* var);

/******************************************************************************
 * literals 
 ******************************************************************************/

//returns the literal with the given index
Lit* sat_index2literal(c2dLiteral index, const SatState* sat_state);

//returns the index of a literal 
c2dLiteral sat_literal2index(const Lit* lit);

//returns the positive literal of a variable
Lit* sat_var2pliteral(const Var* var);

//returns the negative literal of a variable
Lit* sat_var2nliteral(const Var* var);

//returns 1 if the literal is implied, 0 otherwise
//a literal is implied by deciding its variable, or by inference using unit resolution
BOOLEAN sat_is_implied_literal(const Lit* lit);

//returns the weight of the literal (default weight is 1)
//literal weights can be specified in the input cnf file (see above)
c2dWmc sat_literal_weight(const Lit* lit);

//increments the decision level of the given sat state, then sets the literal to true,
//and finally runs unit resolution
//returns a learned clause if unit resolution detected a contradiction, NULL otherwise
Clause* sat_decide_literal(Lit* lit, SatState* sat_state);

//undoes the last literal decision (decrement decision level, un-decide variable, and
//remove all unit-resolution implications)
void sat_undo_decide_literal(SatState* sat_state);

/******************************************************************************
 * clauses
 ******************************************************************************/

//returns the clause with the given index
Clause* sat_index2clause(c2dSize index, const SatState* sat_state);

//returns the index of a clause
c2dSize sat_clause2index(const Clause* clause);

//returns the literals of the clause as an array
Lit** sat_clause2literals(const Clause* clause);

//returns the number of literals in a clause
c2dSize sat_clause_size(const Clause* clause);

//returns 1 if the clause is subsumed, 0 otherwise
BOOLEAN sat_is_subsumed_clause(const Clause* clause);

//returns the number of input clauses of sat state
c2dSize sat_clause_count(const SatState* sat_state);

//returns the number of learned clauses in a sat state
c2dSize sat_learned_clause_count(const SatState* sat_state);

//adds clause to the set of learned clauses, and runs unit resolution
//returns a learned clause if unit resolution finds a contradiction, NULL otherwise
//
//assumes the decision level of the sat state equals to the assertion level of clause
//
//note: this function is called on a clause returned by sat_decide_literal() or sat_assert_clause()
Clause* sat_assert_clause(Clause* clause, SatState* sat_state);

//returns 1 if a clause is marked, 0 otherwise
//all clauses are initially unmarked
BOOLEAN sat_is_marked_clause(const Clause* clause);

//marks a clause
void sat_mark_clause(Clause* clause);

//unmarks a clause
void sat_unmark_clause(Clause* clause);

/******************************************************************************
 * SatState
 ******************************************************************************/

//constructs a sat state from an input cnf file
//all variables and clauses are unmarked when a sat state is created
SatState* sat_state_new(const char* file_name);

//frees the sat state
void sat_state_free(SatState* sat_state);

//applies unit resolution to a sat state that has no decided variables and learned clauses
//returns 1 if unit resolution succeeds, 0 if it finds a contradiction
//
//all variables instantiated by this function are "implied", not "decided," at the
//current decision level. hence, this function does not change the decision level
//of the sat state
//
//note: this function is called immediately after a sat state is created
BOOLEAN sat_assert_unit_clauses(SatState* sat_state);

//undoes sat_assert_unit_clauses(), leading to un-instantiating variables that
//have been instantiated after sat_assert_unit_clauses()
void sat_undo_assert_unit_clauses(SatState* sat_state);

//returns 1 if the decision level of the sat state equals to the assertion level
//of clause, 0 otherwise
//
//note: this function is used to decide whether a learned clause can be asserted
//in a sat state
BOOLEAN sat_at_assertion_level(const Clause* clause, const SatState* sat_state);

#endif //SATAPI_H_

/******************************************************************************
 * end
 ******************************************************************************/
