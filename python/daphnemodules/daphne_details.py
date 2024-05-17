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
import pathlib


class 

class DapheFile :
    """Daphne - Detail  File"""
    def __init__(self) :
        self._map = {}

    def load(self, map_ath : str) -> None :

        map_fp = pathlib.Path(map_path)
        
        # Opening JSON file
        with open(map_fp) as f:
        
            # returns JSON object as 
            # a dictionary
            data = json.load(f)

        self._validate_json(data)
        

        
