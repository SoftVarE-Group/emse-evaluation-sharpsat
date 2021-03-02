# Empirical Evaluation for `Evaluating #SAT Solvers on Industrial Feature Models` (EMSE'21)

This repository provides a benchmark framework and experiments to evaluate #SAT solvers and different knowledge compilers. The experiment design implied by the run configurations can be used to repeat the experiments for our submission to EMSE'21 `Evaluating #SAT Solvers on Industrial Feature Models`.


## How to run

### Run benchmark
In general, a experiment specified by a .json file can be executed with

```
python3 run.py run_configurations/experiment.json
```

## The solvers

For our empirical evaluation, we considered the following solvers/ knowledge compilers.

### DPLL

* [Cachet](https://www.cs.rochester.edu/u/kautz/Cachet/)
* [countAntom](https://projects.informatik.uni-freiburg.de/projects/countantom)
* [Ganak](https://github.com/meelgroup/ganak)
* [PicoSAT](http://fmv.jku.at/picosat/)
* [Relsat](https://code.google.com/archive/p/relsat/)
* [sharpCDCL](http://tools.computational-logic.org/content/sharpCDCL.php)
* [sharpSAT](https://github.com/marcthurley/sharpSAT)

### d-DNNF
* [c2d](http://reasoning.cs.ucla.edu/c2d/)
* [d4](http://www.cril.univ-artois.fr/kc/d4.html)
* [dSharp](https://github.com/QuMuLab/dsharp)

### BDD
* [BuDDy](http://buddy.sourceforge.net/manual/main.html)
* [CNF2OBDD](www.sd.is.uec.ac.jp/toda/code/cnf2obdd.html)
* [Cudd](https://github.com/vscosta/cudd)

### Other
* [CNF2EADT](http://www.cril.univ-artois.fr/kc/eadt.html)
* [miniC2D](http://reasoning.cs.ucla.edu/minic2d/)

### Approximate
* [ApproxMC](https://github.com/meelgroup/approxmc)
* [ApproxCount](https://www.cs.cornell.edu/~sabhar/)
