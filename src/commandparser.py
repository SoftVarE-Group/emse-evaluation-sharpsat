
def buildCommand(commandWithPlaceholders, parameterDictionary):
    return replacePlaceholders(commandWithPlaceholders, parameterDictionary)
    
# replaces placeholders {<placeholder>} with value from dict {<placeholder>: <value>}
def replacePlaceholders(string, replaceDictionary):
    return string.format(**replaceDictionary)


def createParameterDictionary(parameters, defaultParameters):
    if parameters is None:
        return defaultParameters
    if defaultParameters is None:
        return parameters
    paraDict = parameters.copy()
    paraDict.update(defaultParameters)
    return paraDict