#!/usr/bin/env python3

import conffwk
import os
import json
import sys

def add_daphne_conf(oksfile, object_name, json_file, timeout_ms = 1500):
    """Script to add a new DaphneConf object from a given json file"""
    db = conffwk.Configuration("oksconflibs:" + oksfile)

    with open(json_file, 'r') as file:
        data = json.load(file)

    schemafile=f'{os.environ["DAL_SHARE"]}/schema/appmodel/PDS.schema.xml'
    dal = conffwk.dal.module('tutorial', schemafile)

    new_conf = dal.DaphneConf(object_name,
                              timeout_ms=timeout_ms,
                              configuration_file=data)

    db.update_dal(new_conf)
    db.commit()


