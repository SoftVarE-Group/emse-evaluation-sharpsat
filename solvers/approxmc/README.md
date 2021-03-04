[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![build](https://github.com/meelgroup/approxmc/workflows/build/badge.svg)
[![Docker Hub](https://img.shields.io/badge/docker-latest-blue.svg)](https://hub.docker.com/r/msoos/approxmc/)


# ApproxMC4: Approximate Model Counter
ApproxMCv4 is a state-of-the-art approximate model counter utilizing an improved version of CryptoMiniSat to give approximate model counts to problems of size and complexity that were not possible before.

This work is by Kuldeep Meel, Stephan Gocht, and Mate Soos, as [published in AAAI-19](https://www.comp.nus.edu.sg/~meel/Papers/aaai19-sm.pdf) and [in CAV2020](https://www.comp.nus.edu.sg/~meel/Papers/cav20-sgm.pdf). A large part of the work is in CryptoMiniSat [here](https://github.com/msoos/cryptominisat).

ApproxMC handles CNF formulas and performs approximate counting. 

1. If you are interested in exact model counting, visit our exact counter [Ganak](http://github.com/meelgroup/ganak)
2. If you are instead interested in DNF formulas, visit our DNF counter [DNFApproxMC](https://gitlab.com/Shrotri/DNF_Counting).

# ApproxMC5: Sparse-XOR based Approximate Model Counter
Note: this is beta version release, not recommended for general use. We are currently working on a tight integration of sparse XORs into ApproxMC based on our [LICS-20](http://comp.nus.edu.sg/~meel/Papers/lics20-ma.pdf) paper. You can turn on the sparse XORs using the flag "sparse" but beware as reported in LICS-20 paper, this may slow down in some cases; it is likely to give a significant speedup if the number of solutions is very large. 

## Docker image
If you don't have or don't know what an independent set is, first run our MIS tool:
```
docker run --rm -v `pwd`/formula.cnf:/in msoos/mis --timeout 300 /in
[...]
** Copy-paste the following line in the top of your CNF for ApproxMC **
c ind 3 4 7 8 10 11 14 17 18 26 30 35 36 39 42 47 60 62 67 0
```
Then copy-paste that line into your CNF.

Then run the updated CNF through approxmc:
```
cat formula.cnf | docker run --rm -i -a stdin -a stdout msoos/approxmc
```

## How to Build
To build on Linux, you will need the following:
```
sudo apt-get install build-essential cmake
sudo apt-get install zlib1g-dev libboost-program-options-dev libm4ri-dev
```

Then, build CryptoMiniSat and ApproxMC:
```
git clone https://github.com/msoos/cryptominisat
cd cryptominisat
mkdir build && cd build
cmake ..
make
sudo make install
cd ../..
git clone https://github.com/meelgroup/approxmc/
cd approxmc
mkdir build && cd build
cmake ..
make
sudo make install
```

## How to Use
First, you must translate your problem to CNF and just pass your file as input to ApproxMC. Voila -- and it will print the number of solutions of the given CNF formula.

### Sampling Set
For some applications, one is not interested in solutions over all the variables and instead interested in counting the number of unique solutions to a subset of variables, called sampling set. ApproxMC allows you to specify the sampling set using the following modified version of DIMACS format:

```
$ cat myfile.cnf
c ind 1 3 4 6 7 8 10 0
p cnf 500 1
3 4 0
```
Above, using the `c ind` line, we declare that only variables 1, 3, 4, 6, 7, 8 and 10 form part of the sampling set out of the CNF's 500 variables `1,2...500`. This line must end with a 0. The solution that ApproxMC will be giving is essentially answering the question: how many different combination of settings to this variables are there that satisfy this problem? Naturally, if your sampling set only contains 7 variables, then the maximum number of solutions can only be at most 2^7 = 128. This is true even if your CNF has thousands of variables.
### Independent set
For most applications, we are want all solutions to the problem. To do this, you need to use the [MIS](https://github.com/meelgroup/mis) tool to find a small independent set of variables to your CNF. For example, for `formula.cnf` we can do:

```
docker run --rm -v `pwd`/formula.cnf:/in msoos/mis --timeout 300 /in
[...]
** Copy-paste the following line in the top of your CNF for ApproxMC **
c ind 3 4 7 8 10 11 14 17 18 26 30 35 36 39 42 47 60 62 67 0
```

You must copy the line starting with `c ind ...` to the top of your CNF before running ApproxMC.

### Running ApproxMC
In our case, the maximum number of solutions could at most be 2^7=128, but our CNF should be restricting this. Let's see:

```
$ approxmc --seed 5 myfile.cnf
c ApproxMC version 3.0
[...]
c CryptoMiniSat SHA revision [...]
c Using code from 'When Boolean Satisfiability Meets Gauss-E. in a Simplex Way'
[...]
[appmc] using seed: 5
[appmc] Sampling set size: 7
[appmc] Sampling set: 1, 3, 4, 6, 7, 8, 10,
[appmc] Using start iteration 0
[appmc] [    0.00 ] bounded_sol_count looking for   73 solutions -- hashes active: 0
[appmc] [    0.01 ] bounded_sol_count looking for   73 solutions -- hashes active: 1
[appmc] [    0.01 ] bounded_sol_count looking for   73 solutions -- hashes active: 0
[...]
[appmc] FINISHED ApproxMC T: 0.04 s
c [appmc] Number of solutions is: 48*2**1
s mc 96
```
ApproxMC reports that we have approximately `96 (=48*2)` solutions to the CNF's independent support. This is because for variables 3 and 4 we have banned the `false,false` solution, so out of their 4 possible settings, one is banned. Therefore, we have `2^5 * (4-1) = 96` solutions.

### Guarantees
ApproxMC provides so-called "PAC", or Probably Approximately Correct, guarantees. In less fancy words, the system guarntees that the solution found is within a certain tolerance (called "epsilon") with a certain probability (called "delta"). The default tolerance and probability, i.e. epsilon and delta values, are set to 0.8 and 0.2, respectively. Both values are configurable.

### Library usage

The system can be used as a library:

```
#include <approxmc/approxmc.h>
#include <vector>
#include <complex>
#include <cassert>

using std::vector;
using namespace ApproxMC;
using namespace CMSat;

int main() {
    AppMC appmc;
    appmc.new_vars(10);

    vector<Lit> clause;

    //add "-3 4 0"
    clause.clear();
    clause.push_back(Lit(2, true));
    clause.push_back(Lit(3, false));
    appmc.add_clause(clause);

    //add "3 -4 0"
    clause.clear();
    clause.push_back(Lit(2, false));
    clause.push_back(Lit(3, true));
    appmc.add_clause(clause);

    SolCount c = appmc.count();
    uint32_t cnt = std::pow(2, c.hashCount)*c.cellSolCount;
    assert(cnt == std::pow(2, 9));

    return 0;
}
```

### Issues, questions, bugs, etc.
Please click on "issues" at the top and [create a new issue](https://github.com/meelgroup/mis/issues/new). All issues are responded to promptly.

## How to Cite
If you use ApproxMC, please cite the following papers: [CAV20](https://www.comp.nus.edu.sg/~meel/bib/SM19.bib), [AAAI19](https://www.comp.nus.edu.sg/~meel/bib/SM19.bib) and [IJCAI16](https://www.comp.nus.edu.sg/~meel/bib/CMV16.bib).

If you use sparse XORs, please also cite the [LICS20](https://www.comp.nus.edu.sg/~meel/bib/MA20.bib) paper. 

ApproxMC builds on a series of papers on hashing-based approach: [Related Publications](https://www.comp.nus.edu.sg/~meel/publications.html)

The benchmarks used in our evaluation can be found [here](https://www.comp.nus.edu.sg/~meel/Benchmarks/).

## Old Versions
The old version, 2.0 is available under the branch "ver2". Please check out the releases for the 2.x versions under GitHub [releases](https://github.com/meelgroup/approxmc/releases). Please read the README of the old release to know how to compile the code. Old releases should easily compile.
