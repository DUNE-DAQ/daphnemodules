# This module facilitates the generation of daphnemodules DAQModules within daphnemodules apps


# Set moo schema search path                                                                              
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types                                                                                
import moo.otypes
moo.otypes.load_types("daphnemodules/daphnecontroller.jsonnet")

import dunedaq.daphnemodules.daphnecontroller as daphnecontroller

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
#from daqconf.core.conf_utils import Endpoint, Direction

def get_daphnemodules_app(nickname, num_daphnecontrollers, some_configured_value, host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from daphnemodules is generated.
    """

    modules = []

    for i in range(num_daphnecontrollers):
        modules += [DAQModule(name = f"nickname{i}", 
                              plugin = "DaphneController", 
                              conf = daphnecontroller.Conf(some_configured_value = some_configured_value
                                )
                    )]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app
