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
    ret = daphnecontroller.ADCConf(
        resolution    = j['resolution'],
        output_format = j['output_format'],
        SB_first      = j['SB_first'])
    return ret
        

def to_pga( j : dict ) -> daphnecontroller.PGAConf :
    ret = daphnecontroller.PGAConf(
        lpf_cut_frequency  = j['lpf_cut_frequnecy'],
        integrator_disable = j['integrator_disable'],
        gain               = j['gain'] )
    return ret

def to_lna( j : dict ) -> daphnecontroller.LNAConf :
    ret = daphnecontroller.LNAConf(
        clamp              = j['clamp'],
        integrator_disable = j['integrator_disable'],
        gain               = j['gain'] )
    return ret


def get_daphnemodules_app(
                          slot : int,
                          ip : str,
                          biasctrl : int,
                          afe_gain : int,
                          channel_gain : int,
                          channel_offset : int,
                          adc : daphnecontroller.ADCConf,
                          pga : daphnecontroller.PGAConf,
                          lna : daphnecontroller.LNAConf,
                          details,  
                          nickname="daphne",
                          host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from daphnemodules is generated.

    The map file, will profvide details to override whatever comes from the inputs
    """

    ext_conf = details
    
    afes = []
    ext_afe_gains = []
    ext_biases    = []
    ext_adcs      = []
    ext_pgas      = [] 
    ext_lnas      = [] 

    if ext_conf :
        afe_block = ext_conf['afes']
        ext_afe_gains = unpack(afe_block, 'v_gains')
        ext_biases    = unpack(afe_block, 'v_biases')
        ext_adcs      = unpack(afe_block, 'adcs')
        ext_pgas      = unpack(afe_block, 'pgas')
        ext_lnas      = unpack(afe_block, 'lnas')
            
    for afe in range(n_afe) :
        afes.append( daphnecontroller.AFE(
            id=afe,
            v_gain=afe_gain if afe not in ext_afe_gains else ext_afe_gains[afe],
            v_bias = 0      if afe not in ext_biases    else ext_biases[afe],
            adc = adc       if afe not in ext_adcs      else to_adc(ext_adcs[afe]),
            pga = pga       if afe not in ext_pgas      else to_pga(ext_pgas[afe]),
            lna = lna       if afe not in ext_lnas      else to_lna(ext_lnas[afe])
        ) )
        
    channels=[]
    ext_gains = [] 
    ext_offsets = []
    ext_trims =   [] 
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
        slot=slot,
        biasctrl=biasctrl,
        afes = afes,
        channels = channels,
        self_trigger_threshold = 0 if not ext_conf else ext_conf['self_trigger_threshold'],
        full_stream_channels = []  if not ext_conf else ext_conf['full_stream_channels'] 
    )

    modules = [DAQModule(name = "controller", 
                         plugin = "DaphneController", 
                         conf = conf
                         )]

    mgraph = ModuleGraph(modules)
    daphnemodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return daphnemodules_app


