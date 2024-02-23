local moo = import "moo.jsonnet";
local ns = "dunedaq.daphnemodules.daphnecontroller";
local s = moo.oschema.schema(ns);

local types = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",   		   doc="A string"),
    ipaddress: s.string( "IPAddress",              doc="A string containing an IP Address"),   

    daphne_list: s.sequence("DaphneList", self.ipaddress,doc="List of daphne ip addresses to be controlled" ),
  
    conf: s.record("Conf", [
                           s.field("daphne_list", self.daphne_list, 
                                           doc="addresses of the daphne connection point"),
                           ],
                   doc="Configuration for the the Daphne"),

};

moo.oschema.sort_select(types, ns)
