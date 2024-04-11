from rich.console import Console
console = Console()

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes
moo.otypes.load_types("daphnemodules/daphnecontroller.jsonnet")
import dunedaq.daphnemodules.daphnecontroller as daphnecontroller

from appfwk.utils import acmd

import json
import math

def generate_daphne_rc_cmds(
        directory,
        n_samples,
        app_name,
        JSON_DIR,
        DEBUG=False,
    ):

    cmds = [
        ("dump_buffers",    acmd([ ("", daphnecontroller.DumpBuffers(
            directory=directory,
            n_samples=n_samples))])),
        ]

    data_dir = f"{JSON_DIR}/data"

    for c,d in cmds:
        cfg = {        "modules": [
            {"data": {"directory": directory,"n_samples": n_samples},"match": ""}], "entry_state":"CONFIGURED", "exit_state":"CONFIGURED" 
        }
        with open(f"{data_dir}/{app_name}_{c}.json", 'w') as f:
            json.dump(cfg, f, indent=4, sort_keys=True)

