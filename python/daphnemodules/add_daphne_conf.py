#!/usr/bin/env python3

import conffwk
import os
import json
import sys

def add_daphne_conf(oksfile:str, object_name:str, json_file:str, timeout_ms:int = 1500):
    """Script to add a new DaphneConf object from a given json file"""

    print( "Adding file", json_file, "to object", object_name)
    
    db = conffwk.Configuration("oksconflibs:" + oksfile)

    with open(json_file, 'r') as file:
        data = file.read()

    schemafile='schema/appmodel/PDS.schema.xml'
    dal = conffwk.dal.module('dal', schemafile)

    ## create default objects, they will override old configurations
    def_channel = dal.DaphneV2Channel( "daphne-v2-default-channel",
                                       channel_id=100,
                                       gain=1,
                                       offset=2200,
                                       trim=0 )
    db.update_dal(def_channel)

    def_adc = dal.DaphneV2ADC( "daphne-v2-default-adc",
                               low_resolution=False,
                               output_offset_binary=True,
                               MSB_first=True)
    db.update_dal(def_adc)
    
    def_pga = dal.DaphneV2PGA( "daphne-v2-default-pga",
                               lpf_cut_frequency=4,
                               integrator_disable=True,
                               gain=False)
    db.update_dal(def_pga)
    
    def_lna = dal.DaphneV2LNA( "daphne-v2-default-lna",
                               clamp=0,
                               integrator_disable=True,
                               gain=2)
    db.update_dal(def_lna)
    
    def_afe = dal.DaphneV2AFE( "daphne-v2-default-afe",
                               afe_id=100,
                               attenuator=2666,
                               v_bias=0,
                               adc=def_adc,
                               pga=def_pga,
                               lna=def_lna)
    db.update_dal(def_afe)

    def_board = dal.DaphneV2BoardConf( "daphne-v2-default-board",
                                       bias_ctrl=0,
                                       self_trigger_threshold=0,
                                       default_channel=def_channel,
                                       default_afe=def_afe )
    db.update_dal(def_board)
    
    new_conf = dal.DaphneConf(object_name,
                              timeout_ms=timeout_ms,
                              json_file=data,
                              default_v2_settings=def_board )
    db.update_dal(new_conf)
    
    db.commit()


