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
    channel_id: s.number( "ChannelId", "u4",       doc="ChannelID in the [0-40) range"),   

    channel_conf : s.record("ChannelConf", [
    	                                   s.field("offset", self.uint4, 0, doc="Pedestal of the channel"),
				           s.field("gain",   self.uint4, 1, doc="Gain"),
	                                   ], doc = "Channel info" ),

    channel : s.record("Channel", [
 	                          s.field( "id",   self.channel_id, 1000, doc = "id of the properties"),
                                  s.field( "conf", self.channel_conf, doc = "Properties of the specific channel"),
                 	          ], doc = "Configuration coupled with its ID" ),

    channels : s.sequence( "Channels", self.channel,
                           doc = "Configuration for all channels" ),

    afe_conf : s.record( "AFEConf", [
                                    s.field( "reg_52", self.uint4, 0, doc = "value for reg 52 of the AFE, 16 bit register" ),
				    s.field( "reg_4" , self.uint4, 0, doc = "value for reg 4  of the AFE, 5 bit register" ),
				    s.field( "reg_51", self.uint4, 0, doc = "Value for reg 51 of the AFE, 14 bit register" ),
				    s.field( "v_gain", self.uint4, 0, doc = "Value for V gain of the AFE, 12 bit register" ),
				    ], doc="Configuration of the AFE" ),


    afe : s.record( "AFE", [
    	                   s.field( "id", self.channel_id, 1000, doc = "id of the configuration"),
                           s.field( "conf", self.afe_conf, doc = "Configuration of the specific AFE"),
			   ] , doc = "Configuration couples with its ID, ID in [0,5) range" ),
			   
    afes : s.sequence( "AFEs", self.afe, doc="configuration for all AFEs" ),

    conf: s.record("Conf", [
                           s.field("daphne_address", self.ipaddress, 
                                           doc="addresses of the daphne connection point"),
                           s.field("channels", self.channels, 
                                   doc = "Configuration for all the channels") ,
	                   s.field("afes", self.afes,
			           doc = "Configuration for all AFEs" ),
                           ],
                           doc="Configuration for the the Daphne"),

};

moo.oschema.sort_select(types, ns)
