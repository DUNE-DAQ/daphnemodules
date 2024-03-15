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

    adc_conf : s.record( "ADCConf, [
                                 s.field( "resolution",    self.boolean, false, doc="true=12bit, false=14bit"),
                                 s.field( "output_format", self.boolean, true, 
                                          doc="true=Offset Binary, false=2's complement"),
                                 s.field( "SB_first",          self.boolean, true, 
                                          doc="Which Significant bit comes first, true=MSB, false=LSB" ),
                                 ], doc="info to generate Reg4 value" ),

    pga_conf : s.record( "PGAConf, [
                                  s.field( "lpf_cut_frequency", self.uint4,
				           doc="cut frequency, only 4 values acceptable. 0=15MHz, 2=20MHz, 3=30MHz, 4=10MHz"),
                                  s.field( "integrator_disable", self.boolean, true,
				           doc="true=disabled, false=enabled" ),
                                  s.field( "gain",       self.boolean, false, doc="true=30 dB, false=24 dB"),
                                 ], doc="info to generate Reg51 value" ),

    lna_conf : s.record( "LNAConf, [
                                   s.field( "clamp", self.uint4, 0,
				            doc="0=auto setting, 1=1.5 Vpp, 2=1.15 Vpp, 3=0.6 Vpp"),
                                   s.field( "integrator_disable", self.bool, true,
				            doc="true=disabled, false=enabled"),
                                   s.field( "gain", self.uint4, 2,
				            doc="0=18 dB, 1=24 dB, 2=12 dB"),
                                   ], doc="info to generate Reg52 value" ),

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


    channel_list : s.sequence( "ChannelList", self.channel_id, doc="List of channels"),
				  
    full_stream : s.record( "FullStream", [
                            s.field( "channels", self.channel_list,
			             doc="List of channel to be streamed in full stream mode, max 16 channels")
		            ], doc = "Configuration for full stream case" ),

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
