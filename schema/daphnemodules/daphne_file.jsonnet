local moo = import "moo.jsonnet";
local ns = "dunedaq.daphnemodules.daphne_file";
local s = moo.oschema.schema(ns);

local types = {

    int4 :     s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :    s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :     s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :    s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :   s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 :  s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:   s.boolean( "Boolean",                doc="A boolean"),
    string:    s.string(  "String",   		    doc="A string"),
    daphne_id: s.number(  "DaphneId", "u4",         doc="An ID assigned to the daphne module, it is also the slot of the daphe in the crate"),   
    channel_id: s.number( "ChannelId", "u4",        doc="ChannelID in the [0-40) range, [0,5) for the AFE"),   

    gain_entry: s.record("GainEntry", [
    		                      s.field("channel", self.channel_id),
				      s.field("gain",   self.uint4),
				      ], 
				      doc="Specification for a single channel gain entry"),

    gain_map: s.sequence("Gains", self.gain_entry, doc="Specification for channel gains"),
    
    offset_entry: s.record("OffsetEntry", [
    		                          s.field("channel", self.channel_id),
					  s.field("offset",  self.uint4),
				      ], 
				      doc="Specification for a single channel offset entry"),

    offset_map: s.sequence("Offsets", self.offset_entry, doc="Specification for channel offsets"),

    trim_entry: s.record("TrimEntry", [
    		                      s.field("channel", self.channel_id),
				      s.field("trim",    self.uint4),
				      ], 
				      doc="Specification for a single channel trim entry"),

    trim_map: s.sequence("Trim", self.trim_entry, doc="Specification for channel trim"),
    
    channel_conf : s.record("ChannelConf", [
				           s.field("gains",   self.gain_map,   doc="Gains"),
       	                                   s.field("offsets", self.offset_map, doc="Pedestals"),
					   s.field("trims",   self.trim_map,   doc="trims"),
	                                   ], doc = "Channel infos" ),

 
    channel_list : s.sequence( "ChannelList", self.channel_id, doc="List of channels"),

    daphne: s.record( "Daphne", [
				s.field("slot", self.daphne_id, doc="slot used to identify the daphne"),
				s.field("afes", s.any, doc="Block to overrired afe properties"),
				s.field("channels", self.channel_conf, doc="Block to define the channel properties"),
				s.field("self_trigger_threshold", self.uint4, doc="Configuration for full stream case" ),
				s.field("full_stream_channels", self.channel_list,
                                   doc="List of channel to be streamed in full stream mode, max 16 channels. Used only if threshold is 0"), 
                                ], doc="Block to configure a single daphne" ),

};

moo.oschema.sort_select(types, ns)

