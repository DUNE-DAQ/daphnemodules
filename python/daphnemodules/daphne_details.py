"""
Daphne Details file 

BEWARE: Horrible things are done in this module, such that others don't have to suffer

"""

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

moo.otypes.load_types('daphnemodules/daphne_file.jsonnet')
moo.otypes.load_types('daphnemodules/daphnecontroller.jsonnet')

import dunedaq.daphnemodules.daphnecontroller as daphnectrl
import dunedaq.daphnemodules.daphne_file      as daphne_file

import json



