import statistics

def createOutputLine(parameterDict, printParameters, runtimes):
    result = ""
    for printParameter in printParameters:
        result += parameterDict[printParameter]
        result += ","
    if (len(result) >= 1):  #remove last comma if it exists
        result = result[:-1]
    result += ";" + str(statistics.median(runtimes)) + ";" + str(statistics.mean(runtimes)) \
         + ";" + str(min(runtimes)) + ";" + str(max(runtimes)) + "\n"
    return result


def create_raw_output(parameterDict, printParameters, runtimes):
    result = ""
    for printParameter in printParameters:
        result += parameterDict[printParameter]
        result += ","
    if (len(result) >= 1):  #remove last comma if it exists
        result = result[:-1]
    result += ";"
    for runtime in runtimes:
        result += str(runtime) + ","
    result = result[:-1] + "\n"
    return result




    