from ctypes import *

import re
import sys

from .Adapter import *
from utils.IO import log_info, blue, hash_hex
from config import *

name        = "CUDD 3.0.0"
stub        = "cudd"
url         = "https://davidkebo.com/source/cudd_versions/cudd-3.0.0.tar.gz"
archive     = f"{CACHE_DIR}/cudd-3.0.0.tar.gz"
archive_md5 = "4fdafe4924b81648b908881c81fe6c30"
source_dir  = f"{CACHE_DIR}/cudd-3.0.0"
shared_lib  = "libcudd.so"

configure_params = "CFLAGS=-fPIC -std=c99"

hint_install = "--install cudd"

reordering_algorithms = {
    "sift": 4,
    "sift-conv": 5
}

class DdNode(Structure):
    _fields_ = [
        ('index', c_uint),
        ('keys', c_uint)
    ]

class DdSubtable(Structure):
    _fields_ = [
        ('slots', c_uint),
        ('keys', c_uint)
    ]

class DdManager(Structure):
    _fields_ = [
        ('subtables', POINTER(DdSubtable)),
        ('keys', c_uint),
        ('dead', c_uint),
        ('cachecollisions', c_double),
        ('cacheinserts', c_double),
        ('cachedeletions', c_double)
    ]

class STDOUT_Recorder():

    def __init__(self, filename):
        with open(filename, "w"):
            pass

        sys.stdout.flush()
        self._oldstdout_fno = os.dup(sys.stdout.fileno())
        self.sink = os.open(filename, os.O_WRONLY)

    def __enter__(self):
        self._newstdout = os.dup(1)
        os.dup2(self.sink, 1)
        os.close(self.sink)
        sys.stdout = os.fdopen(self._newstdout, 'w')
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.stdout = sys.__stdout__
        sys.stdout.flush()
        os.dup2(self._oldstdout_fno, 1)


class CUDD_Adapter():

    def __init__(self):
        cudd = verify_load_lib(shared_lib, hint_install)

        self.cudd_init = declare(cudd.Cudd_Init, [c_uint, c_uint, c_uint, c_uint, c_ulong], POINTER(DdManager))

        self.cudd_ref = declare(cudd.Cudd_Ref, [POINTER(DdNode)])
        self.cudd_deref = declare(cudd.Cudd_RecursiveDeref, [POINTER(DdManager), POINTER(DdNode)])

        self.cudd_zero = declare(cudd.Cudd_ReadLogicZero, [POINTER(DdManager)], POINTER(DdNode))
        self.cudd_one = declare(cudd.Cudd_ReadOne, [POINTER(DdManager)], POINTER(DdNode))
        self.cudd_ithvar = declare(cudd.Cudd_bddIthVar, [POINTER(DdManager), c_int], POINTER(DdNode))

        self.cudd_and = declare(cudd.Cudd_bddAnd, [POINTER(DdManager), POINTER(DdNode), POINTER(DdNode)], POINTER(DdNode))
        self.cudd_or = declare(cudd.Cudd_bddOr, [POINTER(DdManager), POINTER(DdNode), POINTER(DdNode)], POINTER(DdNode))
        self.cudd_nor = declare(cudd.Cudd_bddNor, [POINTER(DdManager), POINTER(DdNode), POINTER(DdNode)], POINTER(DdNode))

        self.cudd_info = declare(cudd.Cudd_PrintDebug, [POINTER(DdManager), POINTER(DdNode), c_int, c_int])

        self.cudd_enable_dynorder = declare(cudd.Cudd_AutodynEnable, [POINTER(DdManager), c_int])
        self.cudd_disable_dynorder = declare(cudd.Cudd_AutodynDisable, [POINTER(DdManager)])

        self.cudd_n_reorders = declare(cudd.Cudd_ReadReorderings, [POINTER(DdManager)])

        self.cudd_setorder  = declare(cudd.Cudd_ShuffleHeap, [POINTER(DdManager), POINTER(c_uint)])
        self.cudd_bddnewvar = declare(cudd.Cudd_bddNewVar, [POINTER(DdManager)])

        self.cudd_quit = declare(cudd.Cudd_Quit, [POINTER(DdManager)])

    def __enter__(self):
        # From the CUDD manual
        manager = self.cudd_init(0,0, 256, 1<<20, 0)

        self.manager = manager

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        manager = self.manager
        self.cudd_quit(manager)

    def name():
        return name

    def set_dynorder(self, dynorder):
        manager = self.manager
        if dynorder == "off" or dynorder not in reordering_algorithms:
            if dynorder != "off":
                log_warning(f"CUDD: Reordering algorithm not supported ({blue(dynorder)}).")

            log_info(f"CUDD: Disabled automatic reordering")
            self.cudd_disable_dynorder(manager)

        if dynorder in reordering_algorithms:
            log_info(f"CUDD: Enabled automatic reordering ({blue(dynorder)})")
            self.cudd_enable_dynorder(manager, reordering_algorithms[dynorder])

    def cudd_not(self, pointer):

        return byref(pointer.contents, 1)

    @staticmethod
    def install(clean = False):
        if clean:
            log_info(f"Clean installing {blue(name)}")
        else:
            log_info(f"Installing {blue(name)}")

        install_library(name, stub, url, archive, archive_md5, source_dir, shared_lib, configure_params, clean)
        log_info()

    def from_cnf(self, cnf, order, dynorder,filename_bdd):

        filename_bdd = flavour_filename(filename_bdd, stub)

        manager = self.manager

        self.cudd_disable_dynorder(manager)

        # Normalize as CUDD indexes from 0
        order = [x - 1 for x in order] 
        arr = (c_uint * len(order))(*order)

        for x in range(0, cnf.nvars):
            self.cudd_bddnewvar(manager, x)

        self.cudd_setorder(manager, arr)

        self.set_dynorder(dynorder)

        full = self.cudd_one(manager)
        self.cudd_ref(full)
        for n, clause in enumerate(cnf.clauses):
            log_info(clause, f"({n+1} / {len(cnf.clauses)})")

            cbdd = self.cudd_zero(manager)
            self.cudd_ref(cbdd)
            for x in clause:
                var = abs(x) - 1
                f = self.cudd_ithvar(manager, var)

                if x < 0:
                    tmp = self.cudd_or(manager, self.cudd_not(f), cbdd)
                    self.cudd_ref(tmp)
                else:
                    tmp = self.cudd_or(manager, f, cbdd)
                    self.cudd_ref(tmp)

                self.cudd_deref(manager, cbdd)
                cbdd = tmp

            tmp = self.cudd_and(manager, cbdd, full)
            self.cudd_ref(tmp)
            self.cudd_deref(manager, full)
            self.cudd_deref(manager, cbdd)
            full = tmp

        with STDOUT_Recorder(filename_bdd):
            self.cudd_info(manager, full, cnf.nvars, 3)

        self.format_cache(cnf, filename_bdd, order)  
        log_info("BDD saved to", blue(filename_bdd))
        return filename_bdd 

    def format_cache(self, cnf, filename_bdd, order):        
        with open(filename_bdd, "r") as file:
            content = file.read()

        lines = re.split("[\n\r]",content)

        m = re.match(r"^:\s+(?P<n_nodes>\d+)\s+nodes\s+\d+\s+leaves\s+(?P<ssat>[^\s]+)\s+minterms\s*$", lines[0])

        n_nodes = int(m["n_nodes"])
        root = re.split(r"\s+", lines[1])[2]

        root_ce = 0
        if root[0] == '!':
            root_ce = 1
            root = int(root[1:], 16)
        else:
            root = int(root, 16)

        content = [
            f"input_file:{cnf.filename}",
            f"input_hash:{hash_hex(cnf.filename)}",
            f"order:{','.join([str(x + 1) for x in order])}",
            f"n_vars:{cnf.nvars}",
            f"n_nodes:{n_nodes}",
            f"root:{root_ce}:{root}"
        ]

        content.append("----")

        for line in lines[1:]:
            if not line.startswith("ID"):
                continue

            fields = re.split(r"\s+", line)

            if fields[2][0] == "!":
                node_id = int(fields[2][1:], 16)
            else:
                node_id = int(fields[2], 16)

            var_index = int(fields[5])

            high = fields[8]
            high_ce = 0
            if high[0] == '!':
                high_ce = 1
                node_high = int(high[1:], 16)
            else:
                node_high = int(high[0:], 16)

            low = fields[11]
            low_ce = 0
            if low[0] == '!':
                low_ce = 1
                node_low = int(low[1:], 16)
            else:
                node_low = int(low[0:], 16)

            content.append(f"{node_id} {var_index} {low_ce}:{node_low} {high_ce}:{node_high}")

        with open(filename_bdd, "w") as file:
            file.write(f"{os.linesep}".join(content))
            file.write(os.linesep)