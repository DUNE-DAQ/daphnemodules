local moo = import "moo.jsonnet";
local ns = "dunedaq.daphnemodules.daphne_file";
local s = moo.oschema.schema(ns);

local sdaphne = import "daphnemodules/daphnecontroller.jsonnet";
local daphneconf = moo.oschema.hier(sdaphne).dunedaq.daphnemodules.daphnecontroller;


local types = {

    int4 :     s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :    s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :     s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :    s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :   s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 :  s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:   s.boolean( "Boolean",                doc="A boolean"),
    string:    s.string(  "String",   		    doc="A string"),

    
    gain_entry: s.record("GainEntry", [
    		                      s.field("channel", sdaphne.channel_id),
				      s.field("gain",    sdaphne.channel_gain),
				      ], 
				      doc="Specification for a single channel gain entry"),

    gain_map: s.sequence("Gains", self.gain_entry, doc="Specification for channel gains"),
    
    offset_entry: s.record("OffsetEntry", [
    		                          s.field("channel", sdaphne.channel_id),
					  s.field("offset",  sdaphne.offset),
				          ], 
				          doc="Specification for a single channel offset entry"),

    offset_map: s.sequence("Offsets", self.offset_entry, 
                           doc="Specification for channel offsets"),

    trim_entry: s.record("TrimEntry", [
    		                      s.field("channel", sdaphne.channel_id),
				      s.field("trim",    sdaphne.trim),
				      ], 
				      doc="Specification for a single channel trim entry"),

    trim_map: s.sequence("Trim", self.trim_entry, doc="Specification for channel trim"),
    
    channel_conf : s.record("ChannelConf", [
				           s.field("gains",   self.gain_map,   doc="Gains"),
                                           s.field("offsets", self.offset_map, doc="Pedestals"),
		      		           s.field("trims",   self.trim_map,   doc="trims"),
	                                   ], 
					   doc = "Channel infos" ),

    daphne: s.record("Daphne", [
			       s.field("slot",                   sdaphne.slot,      doc="slot used to identify the daphne"),
			       s.field("channels",               self.channel_conf, doc="Block to define the channel properties"),
			       s.field("afes",                   sdaphne.afes,      doc="Block to override daphne AFEs conf"),
			       s.field("self_trigger_threshold", sdaphne.threshold, doc="Configuration for full stream case" ),
			       s.field("full_stream_channels",   sdaphne.channel_list,
                                       doc="List of channel to be streamed in full stream mode, max 16 channels")
                               ], 
                               doc="Block to configure a single daphne" ),

};

sdaphne + moo.oschema.sort_select(types, ns)

