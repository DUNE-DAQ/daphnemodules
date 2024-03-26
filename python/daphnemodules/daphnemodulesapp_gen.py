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
n_afe = 5
n_channels = 4


def get_daphnemodules_app(nickname="daphne",
                          slots : tuple,
                          biasctr : int,
                          afe_gain : int,
                          channel_gain : int,
                          channel_offset : int,
                          adc : daphnecontroller.ADCConf,
                          pga : daphnecontroller.PGAConf,
                          lna : daphnecontroller.LNAConf,
                          map_file = None,  ## for now
                          host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from daphnemodules is generated.
    """

    modules = []

    for s in slots:
        
        ip = ip_base + str(100+s)

        afes = []
        for afe in range(n_afe) :
            afes.append( daphnecontroller.AFE(
                id=afe,
                v_gain=afe_gain,
                v_bias = 0, # or from the map_file
                adc = add,
                pga = pga,
                lna = lna
            ) )

        channels=[]
        for ch in range(n_channels) :
            channels.append( daphnecontroller.Channel(
                id = ch,
                conf = daphnecontroller.ChannelConf(
                    gain = channel_gain,
                    offset = channel_offset
                    # trim from the map
                )
            ) )
            
        conf = daphnecontroller.Conf(
            daphne_address=ip,
            biasctrl=biasctr,
            afes = afes,
            channels = channels,
            self_trigger_threshold = 0,  ## from the map
            full_stream_channels = []  ## from the map
        )

        modules += [DAQModule(name = f"daphne_{s}", 
                              plugin = "DaphneController", 
                              conf = conf
                             )
                    )]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app
