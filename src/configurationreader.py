import json


def parseJsonFile(path):
    with open(path) as json_file:
        contentclass = json.load(json_file)
        return contentclass
