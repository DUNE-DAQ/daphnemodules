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

def unpack( j : dict, block : str )  -> dict :
    ret = {}
    if not j :
        return ret

    for e in j[block] :
        ret[e['id']]=e['value']

    return ret

def to_adc( j : dict ) -> daphnecontroller.ADCConf :

def to_pga( j : dict ) -> daphnecontroller.PGAConf :

def to_lna( j : dict ) -> daphnecontroller.LNAConf :
    

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
        daphnes = unpack(data, 'details')
    
    modules = []

    for s in slots:

        ext_conf = None
        if s in daphnes : ext_conf = daphnes[s]
        
        ip = ip_base + str(100+s)

        afes = []
        if ext_conf :
            afe_block = ext_conf['afes']
            ext_afe_gains = unpack(afe_block, 'v_gains')
            ext_biases    = unpack(afe_block, 'v_biases')
            
        for afe in range(n_afe) :
            afes.append( daphnecontroller.AFE(
                id=afe,
                v_gain=afe_gain if afe not int ext_afe_gains else ext_afe_gains[afe],
                v_bias = 0      if afe not int ext_biases    else ext_biases[afe],
                adc = adc,
                pga = pga,
                lna = lna
            ) )

        channels=[]
        if ext_conf :
            channel_block = ext_conf['channels']
            ext_gains = unpack(channel_block, 'gains')
            ext_offsets = unpack(channel_block, 'offsets')
            ext_trims =   unpack(channel_block, 'trims')
                        
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
            self_trigger_threshold = 0 if not ext_conf else ext_conf['self_trigger_threshold'],
            full_stream_channels = []  if not ext_conf else ext_conf['full_stream_channels'] 
        )

        modules += [DAQModule(name = f"controller_{s}", 
                              plugin = "DaphneController", 
                              conf = conf
                              )
                    ]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app


