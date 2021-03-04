## The solvers

For our empirical evaluation, we considered the following solvers/ knowledge compilers. Note that due to licensing issues we cannot provide each solver in this repository (Cachet, c2d, and approxCount are missing). In this case, you can acccess the solver by the provided links. Several solvers require the GMP Bignumpackage to handle large numbers of satisfying assignments. If binaries do not work, rebuilding them using the source in the provided links may help. If there are any problems with getting the benchmark to run, we gladly receive your feedback and try our best to fix the problems.

### DPLL

* [Cachet](https://www.cs.rochester.edu/u/kautz/Cachet/) (Solver not included here due to license, refer to link)
* [countAntom](https://projects.informatik.uni-freiburg.de/projects/countantom) (Binary provided)
* [Ganak](https://github.com/meelgroup/ganak) (Binary provided)
* [PicoSAT](http://fmv.jku.at/picosat/) (Binary provided)
* [Relsat](https://code.google.com/archive/p/relsat/) (Binary provided)
* [sharpCDCL](http://tools.computational-logic.org/content/sharpCDCL.php)
* [sharpSAT](https://github.com/marcthurley/sharpSAT) (Binary provided)

### d-DNNF
* [c2d](http://reasoning.cs.ucla.edu/c2d/) (Solver not included here due to license, refer to link)
* [d4](http://www.cril.univ-artois.fr/kc/d4.html) (Binary provided)
* [dSharp](https://github.com/QuMuLab/dsharp) (Binary provided)

### BDD
For BuDDy and Cudd, we used the dduerum wrapper which provides the required files to run BuDDy Cudd. Thus, for the benchmark it is sufficient to use dduerum and CNF2OBDD.
* [dduerum] (https://github.com/h3ssto/ddueruem) (Source provided)
* [BuDDy](http://buddy.sourceforge.net/manual/main.html) (Included in dduerum)
* [Cudd](https://github.com/vscosta/cudd) (Included in dduerum)
* [CNF2OBDD](www.sd.is.uec.ac.jp/toda/code/cnf2obdd.html) (Binary provided, bdd_minisat_all_static)

### Other
* [CNF2EADT](http://www.cril.univ-artois.fr/kc/eadt.html) (Solver not included here due to license, refer to link)
* [miniC2D](http://reasoning.cs.ucla.edu/minic2d/) (Source provided)

### Approximate
* [ApproxMC](https://github.com/meelgroup/approxmc) (Source provided)
* [ApproxCount](https://www.cs.cornell.edu/~sabhar/) (Solver not included here due to license, refer to link)
