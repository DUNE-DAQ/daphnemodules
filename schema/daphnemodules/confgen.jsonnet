// This is the configuration schema for daphnemodules

local moo = import "moo.jsonnet";
local nc = moo.oschema.numeric_constraints;

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

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

    daphnemodules: s.record("daphnemodules", [
        s.field( "ip_address", self.string, doc="IP of the daphne"),
    ]),

    daphnemodules_gen: s.record("daphnemodules_gen", [
        s.field("boot", bootgen.boot, default=bootgen.boot, doc="Boot parameters"),
        s.field("daphnemodules", self.daphnemodules, default=self.daphnemodules, doc="daphnemodules Conf parameters"),
    ]),
};

// Output a topologically sorted array.
sboot + moo.oschema.sort_select(cs, ns)
