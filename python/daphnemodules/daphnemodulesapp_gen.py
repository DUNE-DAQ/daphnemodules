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

ip_base = "10.73.137."
allowed_slots = [13]

def get_daphnemodules_app(nickname, slots, host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from daphnemodules is generated.
    """

    modules = []

    for s in slots:
        if s not in allowed_slots :
            raise AttributeError("%s is an invalid slot" % s)
        
        ip = ip_base + str(100+s)
        modules += [DAQModule(name = f"{nickname}_{s}", 
                              plugin = "DaphneController", 
                              conf = daphnecontroller.Conf(
                                  daphne_address=ip,
                                  biasctrl=0,
                                  channels=[],
                                  afes=[],
                                  full_stream_channels=[]
                             )
                    )]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app
