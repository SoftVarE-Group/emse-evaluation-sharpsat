# Replication Package for `Evaluating State-of-the-Art #SAT Solvers on Industrial Configuration Spaces` (EMSE)
[![DOI](https://zenodo.org/badge/342821849.svg)](https://zenodo.org/badge/latestdoi/342821849)



This repository provides a benchmark framework and experiments to evaluate #SAT solvers and different knowledge compilers. The experiment design implied by the run configurations can be used to repeat the experiments for our paper `Evaluating #SAT Solvers on Industrial Feature Models` accepted at the EMSE special issue `Software Product Lines and Variability-rich Systems`.


## How to build

The python benchmark script can be used as it comes.
However prior to executing the benchmark several solvers need to be built. Furthermore, some solvers are not included in this repository due to licensing issues. 
In `solvers/` the solvers are provided either as built binaries, source code, or links to respective repositories.

## How to run

### Run benchmark
In general, an experiment specified by a .json file can be executed with

```
python3 run.py run_configurations/experiment.json
```

## Resources

[Solvers](https://github.com/SoftVarE-Group/emse-evaluation-sharpsat/tree/main/solvers)

[Subject Systems](https://github.com/SoftVarE-Group/emse-evaluation-sharpsat/tree/main/cnf)

## License

The MIT license only applies to src/ run_configurations/ and tools/.
For the solvers, please find the respective licenses in solvers/Licenses. 
