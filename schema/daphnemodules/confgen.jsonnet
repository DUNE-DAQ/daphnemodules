// This is the configuration schema for daphnemodules

local moo = import "moo.jsonnet";
local nc = moo.oschema.numeric_constraints;

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

local sdaphne = import "daphnemodules/DaphneV2ControllerModule.jsonnet";
local daphneconf = moo.oschema.hier(sdaphne).dunedaq.daphnemodules.DaphneV2ControllerModule;

local ns = "dunedaq.daphnemodules.confgen";
local s = moo.oschema.schema(ns);

local cs = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",   		   doc="A string"),   
    monitoring_dest: s.enum(     "MonitoringDest", ["local", "cern", "pocket"]),

    daphne_id : s.record( "DaphneId", [
      s.field("slot", self.uint4,  doc="Slot of the board"),
      s.field("ip",   self.string, doc="IP of the board")
    ], doc="Entry for a map to slot map"),
    
    daphne_list : s.sequence( "DaphneList", self.daphne_id, doc="Map of slots and IPs" ),

    daphne_input: s.record("DaphneInput", [
        s.field( "daphnes", self.daphne_list, default=[],
		 doc="List of the daphne to use, identified by slot"),
	s.field( "timeout_ms", self.uint4, default = 500,
		 doc = "timeout for any interaction with the board; in milliseconds"),
	s.field( "biasctrl", self.uint4, default = 4095,
		 doc = "Biasctr to be used for all boards"),
	s.field( "afe_gain", self.uint4, default = 2667,
		 doc = "Gain to be used for all afes across the boards" ),
	s.field( "channel_gain", self.uint4, default = 2,
		 doc = "Gain to be used for all channels across the boards" ),
	s.field( "channel_offset", self.uint4, default = 1468,
		 doc = "Offset to be used for all channels across the boards" ),
	s.field( "adc", daphneconf.ADCConf, default = daphneconf.ADCConf,
		 doc = "Commond ADC configuration for all the AFEs across the boards" ),
	s.field( "pga", daphneconf.PGAConf, default = daphneconf.PGAConf,
		 doc = "Commond PGA configuration for all the AFEs across the boards" ),
	s.field( "lna", daphneconf.LNAConf, default = daphneconf.LNAConf,
		 doc = "Commond LNA configuration for all the AFEs across the boards" ),
        s.field( "dump_buffers_directory", self.string, default = "./" ,
                 doc ="Where the dump_buffer command will write the spy buffer"),
        s.field( "dump_buffers_n_samples", self.uint8, default = 1024,
                 doc="How many samples to dump")
    ]),

    daphne_gen: s.record("daphne_gen", [
        s.field("boot",   bootgen.boot,      default=bootgen.boot,      doc="Boot parameters"),
        s.field("daphne", self.daphne_input, default=self.daphne_input, doc="daphnemodules Conf parameters"),
    ]),
};

// Output a topologically sorted array.
sboot + sdaphne + moo.oschema.sort_select(cs, ns)
