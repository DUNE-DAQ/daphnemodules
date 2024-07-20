local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.daphnemodules.daphnecontrollerinfo");

local info = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",                 doc="A string"),   

    general_info: s.record("GeneralInfo", [
       s.field("v_bias_0", self.double8, doc="Volt bias for AFE0"),
       s.field("v_bias_1", self.double8, doc="Volt bias for AFE1"),
       s.field("v_bias_2", self.double8, doc="Volt bias for AFE2"),
       s.field("v_bias_3", self.double8, doc="Volt bias for AFE3"),
       s.field("v_bias_4", self.double8, doc="Volt bias for AFE4"),
       s.field("power_minus5v", self.double8, doc="Power(-5V)"),
       s.field("power_plus2p5v", self.double8, doc="Power(2.5V)"),
       s.field("power_ce", self.double8, doc="Power(+CE)"),
       s.field("temperature", self.double8, doc="Temperature in degree celsious"),
    ], doc="monitoring of the Daphne"),

    stream_info : s.record("StreamInfo", [
       s.field("total_packets", self.uint8,  doc="Cumulative counters of sent packages to the felix"),
       s.field("new_packets",   self.uint8, 0,  doc="Incremental value of sent packages to the felix"),
       s.field("total_dropped_packets", self.uint8,  doc="Cumulative counters of  packages NOT sent to the felix"),
       s.field("new_dropped_packets",   self.uint8, 0,  doc="Incremental value of  packages NOT sent to the felix"),
    ], doc="monitoring of the Daphne channels"),


    channel_info : s.record("ChannelInfo", [
       s.field("total_triggers", self.uint8,  doc="Cumulative counters of self trigger windows"),
       s.field("new_triggers",   self.uint8, 0,  doc="Incremental value of the self trigger windows"),
       s.field("total_packets", self.uint8,  doc="Cumulative counters of packets from the FIFO"),
       s.field("new_packets",   self.uint8, 0,  doc="Incremental value of the packets from the FIFO"),
    ], doc="monitoring of the Daphne channels")
};

moo.oschema.sort_select(info)
