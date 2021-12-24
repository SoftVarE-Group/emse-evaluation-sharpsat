#!/usr/bin/env python2

from pypsdd import Vtree,SddManager
from pypsdd import io

vtree_filename = "tmp/ucla.vtree"
sdd_filename = "tmp/ucla.sdd"

vtree = Vtree.read(vtree_filename)
manager = SddManager(vtree)
alpha = io.sdd_read(sdd_filename,manager)
mc = alpha.model_count(vtree)
print "s mc %d" % mc
