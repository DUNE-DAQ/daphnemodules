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

import json

ip_base = "10.73.137."
n_afe = 5
n_channels = 40


def get_daphnemodules_app(
                          slots : tuple,
                          biasctrl : int,
                          afe_gain : int,
                          channel_gain : int,
                          channel_offset : int,
                          adc : daphnecontroller.ADCConf,
                          pga : daphnecontroller.PGAConf,
                          lna : daphnecontroller.LNAConf,
                          map_file,  
                          nickname="daphne",
                          host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from daphnemodules is generated.

    The map file, will profvide details to override whatever comes from the inputs
    """

    daphnes = {}
    if map_file :
        file = open(map_file)
        data = json.load(file)
        for c in data['details'] :
            daphnes[c['slot']] = c['conf']
    
    modules = []

    for s in slots:

        ext_conf = None
        if s in daphnes : ext_conf = daphnes[s]
        
        ip = ip_base + str(100+s)

        afes = []
        for afe in range(n_afe) :
            afes.append( daphnecontroller.AFE(
                id=afe,
                v_gain=afe_gain,
                v_bias = 0, # or from the map_file
                adc = adc,
                pga = pga,
                lna = lna
            ) )

        channels=[]
        ext_gains = {}
        ext_offsets = {}
        ext_trims = {}
        if  ext_conf :
            ext_channels = ext_conf['channels']
            for g in ext_channels['gains'] :
                ext_gains[g['channel']] = g['gain']
            for o in ext_channels['offsets'] :
                ext_offsets[o['channel']]=o['offset']
            for t in ext_channels['trims'] :
                ext_trims[t['channel']]=t['trim']
                        
        for ch in range(n_channels) :
            conf = None

            gain = channel_gain     if ch not in ext_gains   else ext_gains[ch]
            offset = channel_offset if ch not in ext_offsets else ext_offsets[ch]
            if ch in ext_trims :
                conf = daphnecontroller.ChannelConf(
                    gain = gain,
                    offset = offset, 
                    trim = ext_trims[ch] )
            else :
                conf = daphnecontroller.ChannelConf(
                    gain = gain, 
                    offset = offset )

            channels.append( daphnecontroller.Channel( id = ch, conf = conf ) ) 
            
        conf = daphnecontroller.Conf(
            daphne_address=ip,
            biasctrl=biasctrl,
            afes = afes,
            channels = channels,
            self_trigger_threshold = 0,  ## from the map
            full_stream_channels = []  ## from the map
        )

        modules += [DAQModule(name = f"controller_{s}", 
                              plugin = "DaphneController", 
                              conf = conf
                              )
                    ]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app
