# ddueruem
A wrapper for the BDD libraries BuDDy and CUDD.

### Requirements
* Python 3.8+
* `make`
* `glibc`

### Usage:
```bash
# create virtual environment (Python 3.x)
python -m venv .venv

# activate venv
source .venv/bin/activate

# install required packages
pip install -r requirements.txt

# show ddueruem help
./ddueruem -h

# install BuDDy & CUDD
./ddueruem --install buddy cudd

# create BDD for example sandwich.dimacs
./ddueruem --buddy examples/sandwich.dimacs
./ddueruem --cudd examples/sandwich.dimacs

# preorder with FORCE
./ddueruem examples/sandwich.dimacs --preorder force

# Ignore a previously cached variable order
./ddueruem examples/sandwich.dimacs --preorder force --ignore-cache

# Disable automatic reordering
./ddueruem examples/sandwich.dimacs --dynorder off
```

### Defaults:
* **Default lib:** BuDDy
* **Preorder:** off
* **Dynorder:** sift-converge

### Troubleshooting
1) `bash: ./ddueruem.py: Permission denied` <br>
**Solution:** Execute `chmod u+x ddueruem.py` to make the file executable.
