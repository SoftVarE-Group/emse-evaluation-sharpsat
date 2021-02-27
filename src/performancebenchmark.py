# Computes time required for a command line process

from src.configurationreader import parseJsonFile
from src.commandparser import buildCommand, createParameterDictionary
from src.outputparser import createOutputLine, create_raw_output
import src.constant as const

from itertools import zip_longest
import time
import timeit
import subprocess
import os
import resource
import sys


timeit.template = """
def inner(_it, _timer{init}):
    {setup}
    _t0 = _timer()
    for _i in _it:
        retval = {stmt}
    _t1 = _timer()
    return _t1 - _t0, retval
"""


def run_benchmark(config_file_path):
    configuration = parseJsonFile(config_file_path)
    create_folder_if_not_exists(configuration[const.OUTPUT_DIR])
    for calls in configuration[const.CMD_CALLS]:
        cmd_file, raw_file = create_files(configuration[const.OUTPUT_DIR], calls[const.NAME])
        if (configuration[const.SHOW_PROGRESS]):
            print("---------------- Evaluating: " +
                calls[const.NAME] + "----------------")
        for instance in calls[const.INSTANCES]:
                handle_instance(instance, calls, configuration, cmd_file, raw_file)
        cmd_file.close()
        raw_file.close()


def handle_instance(instance, calls, configuration, cmd_file, raw_file):
    default_dict = instance[const.DEFAULT_PARAMETERS]
    for parameters, repeatingparameters in zip_longest(instance[const.PARAMETERS], configuration[const.REPEATING_PARAMETERS]):
        parameter_dictionary = createParameterDictionary(
            parameters, repeatingparameters)
        parameter_dictionary = createParameterDictionary(
            parameter_dictionary, default_dict)
        command = buildCommand(calls[const.COMMAND], parameter_dictionary)
        if (configuration[const.SHOW_PROGRESS]):
            print("Evaluating: " + command + "...")
        runtimes = []
        for i in range (configuration[const.NUMBER_OF_RUNS]):
            runtimes.append(measure_runtime(command, configuration[const.MAX_MEMORY], \
                 configuration[const.TIMEOUT], configuration[const.SUPPRESS_OUTPUT]))
        cmd_file.write(createOutputLine(
                    parameter_dictionary, calls[const.PRINT_PARAMETERS], runtimes))
        raw_file.write(create_raw_output(parameter_dictionary, calls[const.PRINT_PARAMETERS], runtimes))
    return runtimes, parameter_dictionary



def create_files(output_dir, command):
        file_path = os.path.join(output_dir, command + ".output")
        raw_filepath = os.path.join(output_dir, command + "_raw.output")
        if os.path.isfile(file_path) or os.path.isfile(raw_filepath):
            sys.exit("File already exists.")
        
        return open(file_path, "w+"), open (raw_filepath, "w+")



def measure_runtime(command, max_memory, timeout, suppress_output):
    setup = ("import subprocess, os\n"
               "from src.memory_management import limit_virtual_memory")
    command_array = command.split(" ")
    result = 0
    output_pipe_string = ", stdout=subprocess.PIPE" if suppress_output else ""
    try:
        # subprocess.PIPE is used to suppress the output of the command call
        t = timeit.Timer(stmt="subprocess.run({}, preexec_fn=limit_virtual_memory({}), timeout={}{}, stderr = subprocess.PIPE)".format(
            command_array, max_memory, timeout, output_pipe_string), setup=setup)
        result = t.timeit(number=1)
    except subprocess.TimeoutExpired as expired:
        print(expired)
        return timeout
    if (result[1].returncode < 0):
        return result[1].returncode
    if (result[0] > timeout):
        print(command + 'reached timeout')
        return timeout
    return result[0]



def create_folder_if_not_exists(name):
    try:
        os.makedirs(name)
    except FileExistsError:
        pass

