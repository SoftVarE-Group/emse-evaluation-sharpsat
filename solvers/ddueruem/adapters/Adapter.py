from ctypes import CDLL

import os
from os import path

import re

from utils.IO import blue, red, green, download, untar, verify_hash, log_info, log_error

import subprocess

def declare(f, argtypes, restype = None):
    x = f
    x.argtypes  = argtypes

    if restype:
        x.restype   = restype

    return x

def verify_load_lib(lib, hint_install):
    if not path.exists(lib):
        log_error(red(lib), "not found, please install first with", blue(hint_install))
        exit(1)
    else:
        return CDLL(f"./{lib}")


def flavour_filename(filename, stub):
    filename = re.sub(".dd$", f"-{stub}.dd", filename)
    return filename

def install_library(name, stub, url, archive, archive_md5, sources, shared_lib, configure_params = "", clean = False):

    if path.exists(shared_lib):
        if clean:
            log_info(f"Ignoring existing shared library ({blue(shared_lib)})")
        else: 
            log_info(blue(shared_lib), "already exists, skipping install")
            return 

    if clean or not path.exists(archive):    
        log_info("Downloading...")
        download(url, archive)

    if clean or not path.exists(sources):        
        valid, reason = verify_hash(archive, archive_md5)

        if not valid:
            log_error(reason)

        untar(archive)

    if clean or not path.exists(shared_lib):            
        log_info("Configuring...")
        subprocess.run(['./configure', configure_params], cwd = sources, stdout=subprocess.PIPE).stdout.decode('utf-8')

        log_info("Building...")
        subprocess.run(['make', stub, '-j4'], stdout=subprocess.PIPE).stdout.decode('utf-8')

    if path.exists(shared_lib):        
        if path.exists(shared_lib):
            log_info(f"{name} build:", green("SUCCESS", True))
        else:
            log_info(f"{name} build:",  red("FAIL", True))