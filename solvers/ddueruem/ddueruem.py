#!/usr/bin/env python3
import argparse

from copy import copy

from ctypes import *
import ctypes

from datetime import datetime, timedelta

import hashlib

import os
from os import path

from random import shuffle
import re
import requests

import subprocess

from termcolor import colored

import tarfile

from adapters import BUDDY, CUDD
import config

from utils.VariableOrdering import compute_default_order, force, force_triage, sort_clauses_by_span
from utils.IO import log_info, log_warning, log_error, blue, verify_hash, hash_hex

class CNF:

    def __init__(self, filename, nvars, var_descs, clauses):
        self.filename = filename
        self.nvars = nvars
        self.var_descs = var_descs
        self.clauses = clauses


DYNORDER_CHOICES = ["off", "sift", "sift-conv"]
INSTALL_CHOICES = ["buddy", "cudd", "sylvan"]
PREORDER_CHOICES = ["force", "force-triage"]


CLI_HELP = {
    "file": "The DIMACS file to construct the BDD for",

    "--silent": "Disable all output (libraries may have unsuppressible output).",
    "--verbose": "Enable verbose output.",

    "--buddy": "Uses BuDDy to build the BDD.",
    "--cudd": "Uses CUDD to build the BDD. (Default)",

    "--preorder": "The preorder algorithm to use",
    "--ignore-cache": "Do not use cached variable order or BDD",

    "--dynorder": "The dynamic reordering algorithm to use",

    "--install": "Download and install the chosen libraries.",
    "--clean-install": "Forces download and install of the chosen libraries."
}

#

def parse_dimacs(filename):

    var_descs = {}

    nvars = 0
    nclauses = 0

    clauses = []

    with open(filename) as file:
        for line in file.readlines():
            m = re.match(r"(?P<type>[cp]) (?P<content>.*)", line)

            if m is None:
                line = re.sub(r"[\n\r]", "", line)
                clause = re.split(r"\s+", line)
                clause = [int(x) for x in clause if x != '0']
                
                clauses.append(clause)

            elif m["type"] == "c":
                m = re.match(r"\s*(?P<id>[1-9][0-9]*) (?P<desc>\w+)", m["content"])

                if m is not None:
                    var_descs[m["id"]] = m["desc"]

            elif m["type"] == "p":
                m = re.match(r"\s*(?P<type>\w+) (?P<nvars>\d+) (?P<nclauses>\d+)", m["content"])

                if m["type"] != "cnf":
                    print(f"[ERROR] Only CNFs are supported at this point, but type is ({m['type']})")

                nvars = int(m["nvars"])
                nclauses= int(m["nclauses"])

    if nclauses != len(clauses):
        print(f"[WARNING] Specified number of clauses ({nclauses}) differs from number of parsed ones ({len(clauses)}).")

    return CNF(filename, nvars, var_descs, clauses)

###
def get_lib(lib_name):
    if lib_name == "buddy":
        return BUDDY
    elif lib_name == "cudd":
        return CUDD

    return None

def read_cache_from_file(filename):

    with open(filename) as file:
        content = file.read()

    lines = re.split("[\n\r]", content)

    data = {}

    for line in lines:
        raw = re.split(":", line)
        key = raw[0]
        value = raw[1:]
        data[key] = value



    # sanitize order
    if "order" in data:
        data["order"] = [int(x) for x in re.split(r",", data["order"][0])]

    return data

def validate_cache(cache, filename):
    if not cache:
        return (False, "Cache damaged")

    if not "file_md5" in cache:
        return (False, "Cache contains no hash")

    hash_should = cache["file_md5"][0]

    return verify_hash(filename, hash_should)

def satcount(root_ce, root, nodes, cnf):
    count = 0

    stack = []
    stack.append((root, 0, root_ce == 1))

    while stack:
        node, depth, complemented = stack.pop()

        if (node == 0 and complemented) or (node == 1 and not complemented):
            count += 1 << (cnf.nvars - depth)
        elif node != 0 and node != 1:
            _, low_ce, low, high_ce, high = nodes[node]

            stack.append((low, depth +1, complemented ^ low_ce))
            stack.append((high, depth +1, complemented ^ high_ce))

    return count

def run_preorder(cnf, preorder):

    if preorder == "force":
        order, span = force(cnf)
    elif preorder == "force-triage":        
        log_info()
        log_info("Beginning variable ordering with triaged FORCE")
        order, span = force_triage(cnf)
        log_info("Triaged FORCE has finished")
    else:
        log_warning(blue(preoder), "currently not implemented")
        order = compute_default_order(cnf)

    return order

def run_lib(lib, cnf, order, dynorder, filename_bdd):
    log_info("Building BDD for", colored(cnf.filename, "blue"), "using", f"{colored(lib.name(), 'blue')}.")

    with lib() as bdd:
        filename_bdd = bdd.from_cnf(cnf, order, dynorder, filename_bdd)

    return filename_bdd

def report(cnf, filename_bdd, filename_report_ts, filename_report):

    if not path.exists(filename_bdd):
        log_error("No cached BDD found for", colored(cnf.filename, "blue"))
        exit(1)

    with open(filename_bdd) as file:
        lines = re.split("[\n\r]", file.read())

    m = 0

    data = {}

    for i, line in enumerate(lines):
        if line == "----":
            m = i
            break

        line = re.split(":", line)
        data[line[0]] = line[1:]

    if not "root" in data:
        log_error(f"Cannot compile report, as {filename_bdd} is broken (root missing)")
        exit(1)
    else:
        root_ce = int(data["root"][0])
        root = int(data["root"][1])

    if not "n_nodes" in data:
        log_error(f"Cannot compile report, as {filename_bdd} is broken (n_nodes missing)")
        exit(1)
    else:
        n_nodes = int(data["n_nodes"][0])

    if "order" in data:
        order = data["order"]
    else:
        order = ""

    nodes = {}
    for line in lines[m+1:]:
        m = re.match(r"(?P<id>\d+) (?P<var>\d+) (?P<low_ce>\d):(?P<low>\d+) (?P<high_ce>\d):(?P<high>\d+)", line)
        if m:
            nodes[int(m["id"])] = (int(m["var"]), int(m["low_ce"]) == 1, int(m["low"]), int(m["high_ce"]) == 1, int(m["high"]))

    ssat = satcount(root_ce, root, nodes, cnf)

    print("--------------------------------")
    print(f"Results for {colored(cnf.filename, 'blue')}:")
    print("#SAT:", ssat, sep = "\t")
    print("Nodes:", n_nodes, sep = "\t")
    print("--------------------------------")

    contents = [
        f"file:{cnf.filename}",
        f"file_md5:{hash_hex(cnf.filename)}",
        f"bdd:{filename_bdd}",
        f"bdd_md5:{hash_hex(filename_bdd)}",
        f"order:{','.join([str(x) for x in order])}",
        f"#SAT:{ssat}",
        f"#nodes:{n_nodes}"
    ]

    content = os.linesep.join(contents)

    with open(filename_report_ts, "w") as file:
        file.write(content)
        file.write(os.linesep)

    with open(filename_report, "w") as file:
        file.write(content)
        file.write(os.linesep)

def report_order(cnf, order, filename_report):

    contents = [
        f"file:{cnf.filename}",
        f"file_md5:{hash_hex(cnf.filename)}",
        f"order:{','.join([str(x) for x in order])}"
    ]

    content = os.linesep.join(contents)

    with open(filename_report, "w") as file:
        file.write(content)
        file.write(os.linesep)

def run(filename, lib, preorder, dynorder, caching):

    if not path.exists(filename):
        log_error(f"Could not find", blue(filename),f"aborting.{os.linesep}Either give the absolute path to the file or the relative path wrt", blue(os.path.realpath(__file__)))
        exit(1)

    filename_base = os.path.basename(filename).split('.')[0]
    filename_report = f"{config.REPORT_DIR}/{filename_base}.ddrep"
    filename_report_ts = f"{config.REPORT_DIR}/{filename_base}-{datetime.now().strftime('%Y%m%d%H%M%S')}.ddrep"
    filename_bdd = f"{config.CACHE_DIR}/{filename_base}.dd" 

    cnf = parse_dimacs(filename)

    #FIXME Read from report file (date wildcard)
    cache = None
    if path.exists(filename_report) and caching:
        log_info(f"Found cache for", blue(filename), f"({os.path.relpath(filename_report)})")
        cache = read_cache_from_file(filename_report)
        valid, reason = validate_cache(cache, filename)

        if not valid:
            log_warning(reason)
            cache = None

    order = None
    if cache:
        if "order" in cache:
            log_info("Found", blue("variable order"), "in cache")
            order = cache["order"]
        else:
            log_info("Cache contains no", blue("variable order"))

    computed_order = False
    if order and preorder:
        log_warning("Ignoring flag", blue(f"--preorder {preorder}"), "as cache exists and flag", blue("--ignore-cache"), "was not supplied")
    else:
        if order is None:
            if preorder is None:
                order = compute_default_order(cnf)
            else:
                computed_order = True
                order = run_preorder(cnf, preorder)

    if computed_order:
        report_order(cnf, order, filename_report)

    cnf.clauses = sort_clauses_by_span(cnf.clauses, order)

    filename_bdd = run_lib(lib, cnf, order, dynorder, filename_bdd)
    report(cnf, filename_bdd, filename_report_ts, filename_report)


def cli():

    lib_default = config.LIB_DEFAULT

    parser = argparse.ArgumentParser(description="Wrapper for BuDDy and CUDD.")
    parser.add_argument("file", nargs = "?", help = CLI_HELP["file"], default = None)

    # IO options
    parser.add_argument("--silent", help = CLI_HELP["--silent"], dest = "silent", action = "store_true", default = False)
    parser.add_argument("--verbose", help = CLI_HELP["--verbose"], dest = "verbose", action = "store_true", default = False)

    # Run options
    parser.add_argument("--buddy", help = CLI_HELP["--buddy"], dest = "lib", action = "store_const", const = "buddy", default = lib_default)
    parser.add_argument("--cudd", help = CLI_HELP["--cudd"], dest = "lib", action = "store_const", const = "cudd", default = lib_default)

    # Preorder
    parser.add_argument("--preorder", help = CLI_HELP["--preorder"], choices = PREORDER_CHOICES, type = str.lower, default = None)
    parser.add_argument("--ignore-cache", help =CLI_HELP["--ignore-cache"], dest = "caching", action = "store_false", default = True)

    # Reorder
    parser.add_argument("--dynorder", help = CLI_HELP["--dynorder"], choices = DYNORDER_CHOICES, type = str.lower, default = "sift-conv")

    # Install options
    parser.add_argument("--install", nargs = "+", choices = INSTALL_CHOICES, type = str.lower, help = CLI_HELP["--install"], default = [])
    parser.add_argument("--clean-install", nargs = "+", choices = INSTALL_CHOICES, type = str.lower, help = CLI_HELP["--clean-install"], default = [])


    args = parser.parse_args()

    # Perform clean installs:
    libs_clean_install = args.clean_install
    libs_install = args.install

    if libs_clean_install or libs_install:
        config.verbose = True
        log_info("Install requested, ignoring remaining parameters and flags")

    for lib in libs_clean_install:
        lib = get_lib(lib)
        lib.install(clean = True)

    # Perform  installs:
    for lib in libs_install:
        lib = get_lib(lib)
        lib.install(clean = False)

    if libs_clean_install or libs_install:
        exit(0)

    # Set toggles
    config.verbose = args.verbose
    config.silent = args.silent

    if config.verbose and config.silent:
        log_warning("Both", blue("--verbose"), "and", blue("--silent"), "are present, ignoring both.")
        config.verbose = False
        config.silent = False

    filename = args.file
    preorder = args.preorder
    caching  = args.caching
    dynorder = args.dynorder

    lib = get_lib(args.lib)

    run(filename, lib, preorder, dynorder, caching)

def init():    
    os.chdir(os.path.dirname(os.path.realpath(__file__)))

    # check if the .cache directory exists and create it otherwise
    if not path.exists(".cache"):
        try:
            os.mkdir(".cache")
        except OSError as ose:
            log_error("[ERROR] Creating of directory", colored(".cache", "blue"), "failed")
        else:
            log_info(f"Created cache directory at {path.abspath('.cache')}")

    if not path.exists("reports"):
        try:
            os.mkdir("reports")
        except OSError as ose:
            log_error("[ERROR] Creating of directory", colored("reports", "blue"), "failed")
        else:
            log_info(f"Created reports directory at {path.abspath('reports')}")

init()
cli()
