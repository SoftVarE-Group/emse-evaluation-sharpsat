from datetime import datetime, timedelta

import hashlib

import os

import requests

import tarfile
from termcolor import colored

import config

def blue(msg, bold = False, attrs = []):
    if bold:
        attrs.append("bold")

    return colored(msg, "blue", attrs = attrs)

def red(msg, bold = False, attrs = []):
    if bold:
        attrs.append("bold")
    return colored(msg, "blue", attrs = attrs)

def green(msg, bold = False, attrs = []):
    if bold:
        attrs.append("bold")

    return colored(msg, "green", attrs = attrs)

def log_error(msg, *msgs):    
    print()
    print(colored("[ERROR]", "red", "on_white", attrs = ["bold"]), msg, *msgs)
    print()

def log_info(msg = None, *msgs):
    if msg is None:
        print()
        return

    if config.verbose:
        print(f"[{colored(datetime.now().strftime('%Y-%m-%d:%H-%M-%S'), 'blue')}]", msg, *msgs)

def log_warning(msg, *msgs):
    print(colored("[WARNING]", "yellow", attrs = ["bold"]), msg, *msgs)

### Download

def untar(filepath):
    log_info(f"Unpacking {filepath}")

    with tarfile.open(filepath) as archive:
        def is_within_directory(directory, target):
            
            abs_directory = os.path.abspath(directory)
            abs_target = os.path.abspath(target)
        
            prefix = os.path.commonprefix([abs_directory, abs_target])
            
            return prefix == abs_directory
        
        def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
        
            for member in tar.getmembers():
                member_path = os.path.join(path, member.name)
                if not is_within_directory(path, member_path):
                    raise Exception("Attempted Path Traversal in Tar File")
        
            tar.extractall(path, members, numeric_owner=numeric_owner) 
            
        
        safe_extract(archive, path=config.CACHE_DIR)

def download(url, target):

    req = requests.get(url)

    with open(target, "wb") as file:
        file.write(req.content)

### Hashing

def hash_hex(filepath):    
    with open(filepath, "rb") as f:
        hash_is = hashlib.md5()
        while chunk := f.read(8192):
            hash_is.update(chunk)

    return hash_is.hexdigest()


def verify_hash(filepath, hash_should):

    hash_is = hash_hex(filepath)

    if hash_is == hash_should:
        return (True, "")
    else:
        return (False, f"Hash of {filepath} ({hash_is}) does not match expected hash ({hash_should})")

def prepend_input_file_signature(filename_to_hash, filename_to_store):
    with open(filename_to_store, "r") as file:
        contents = file.read()

    with open(filename_to_store, "w") as file:
        file.write(f"{filename_to_hash}:{hash(filename_to_hash)}{os.linesep}")
        file.write(contents)