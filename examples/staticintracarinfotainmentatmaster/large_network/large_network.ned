//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga, Rudolf Hornig
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//
package core4inet.examples.staticintracarinfotainmentatmaster.large_network;
import ned.DatarateChannel;

import inet.linklayer.contract.IWiredNic;
import inet.node.inet.INetworkNode;
import core4inet.applications.base.IApplication;

import core4inet.optical.MiddleLayer;
import core4inet.optical.OpBGTrafficSourceApp;
import core4inet.optical.OpBGTrafficSourceApp2;
import core4inet.optical.OpBGTrafficSinkApp;

import inet.linklayer.ppp.PPPInterface;

//import core4inet.applications.trafficsource.base.BGTrafficSourceApp;

simple OpNode
{
    parameters:
        @networkNode;
        
        int datachannels = 8;
        int channelallocationtype = 0;
        int gatewaycount=5;
        double slotsize=default(1500);
        double guard @unit(s)=default(100ns);
        double video_packet_period @unit(s)=default(1s);
        @signal[arrival](type="long");
        @statistic[hopCount](title="hop count"; source="arrival"; record=vector,stats; interpolationmode=none);

        @display("i=block/routing");
    gates:
        
        input control_in;
        input data_in[];
        output control_out;
        output data_out[];
        inout gate[];

        
}

module EndHost
{
    parameters:
        @networkNode;
        
        int numSourceApps = default(0);
        int numSourceApps2 = default(0);
        int numSinkApps = default(0);
        
        

    gates:
        
        inout ethg @labels(EtherFrame-conn);


        
    submodules:    
        eth: PPPInterface  {
            parameters:
                @display("p=368,406,row,60;q=txQueue");
        }
        middlelayer: MiddleLayer ;
        
        
        // Applications for the device
        AppSource[numSourceApps]:  OpBGTrafficSourceApp {
            @display("p=400,50,row,100");
        }
        
        AppSource2[numSourceApps2]:  OpBGTrafficSourceApp2 {
            @display("p=400,50,row,100");
        }
        
        
        AppSink[numSinkApps]:  OpBGTrafficSinkApp {
            @display("p=400,50,row,100");
        }

        
    connections allowunconnected:    
    	
            ethg <--> { @display("m=s"); } <--> eth.phys;

            
            eth.upperLayerOut --> middlelayer.NetIn;
            eth.upperLayerIn <-- middlelayer.NetOut;
            
            for i=0..numSourceApps-1 {
                middlelayer.AppIn++ <-- AppSource[i].Out;
        	}
        	
        	for i=0..numSourceApps2-1 {
                middlelayer.AppIn++ <-- AppSource2[i].Out;
        	}
        	
        	for i=0..numSinkApps-1 {
                middlelayer.AppOut++ --> AppSink[i].In;
        	}
            
            

        
}

network large_network
{
    parameters:
        @figure[description](type=text; pos=5,20; font=,,bold; 
        	text="Optical IntraVehicle Backbone Simulation");

    types:
        channel Channel extends ned.DelayChannel {
            delay = 20ns;
        }
        channel ethline extends DatarateChannel
        {
            delay = 20ns;
            datarate = 105Gbps;
        }
        
        
    submodules:
        tic[6]: OpNode;
        
        
        node_1: EndHost;
        node_2: EndHost;
        node_3: EndHost;
        node_4: EndHost;
        node_5: EndHost;
        node_6: EndHost;
        node_7: EndHost;
        node_8: EndHost;
        node_9: EndHost;
        node_10: EndHost;
        node_11: EndHost;

        
    connections:

        //for i=0..5 {
          //  tic[i].control_out --> Channel  --> tic[(i+1)%6].control_in;
            //tic[i].data_out++ --> Channel  --> tic[(i+1)%6].data_in++;
           // }
            
            tic[0].control_out --> Channel  --> tic[1].control_in;
            tic[1].control_out --> Channel  --> tic[2].control_in;
            tic[2].control_out --> Channel  --> tic[3].control_in;
            tic[3].control_out --> Channel  --> tic[4].control_in;
            tic[4].control_out --> Channel  --> tic[5].control_in;
            tic[5].control_out --> Channel  --> tic[0].control_in;
            
            tic[0].data_out++ --> Channel  --> tic[1].data_in++;
            tic[1].data_out++ --> Channel  --> tic[2].data_in++;
            tic[2].data_out++ --> Channel  --> tic[3].data_in++;
            tic[3].data_out++ --> Channel  --> tic[4].data_in++;
            tic[4].data_out++ --> Channel  --> tic[5].data_in++;
            tic[5].data_out++ --> Channel  --> tic[0].data_in++;

        
        node_1.ethg$i <-- ethline <-- tic[4].gate$o++;
        node_1.ethg$o --> ethline --> tic[4].gate$i++;
        
        node_2.ethg$i <-- ethline <-- tic[5].gate$o++;
        node_2.ethg$o --> ethline --> tic[5].gate$i++;
        
        node_3.ethg$i <-- ethline <-- tic[0].gate$o++;
        node_3.ethg$o --> ethline --> tic[0].gate$i++;
        
        node_4.ethg$i <-- ethline <-- tic[0].gate$o++;
        node_4.ethg$o --> ethline --> tic[0].gate$i++;
        
        node_9.ethg$i <-- ethline <-- tic[0].gate$o++;
        node_9.ethg$o --> ethline --> tic[0].gate$i++;
        
        //to connect to the master
        //node_10.ethg$i <-- ethline <-- tic[0].gate$o++;
        //node_10.ethg$o --> ethline --> tic[0].gate$i++;
        
        
        
        
        node_5.ethg$i <-- ethline <-- tic[1].gate$o++;
        node_5.ethg$o --> ethline --> tic[1].gate$i++;
        
        node_6.ethg$i <-- ethline <-- tic[2].gate$o++;
        node_6.ethg$o --> ethline --> tic[2].gate$i++;
        
        node_11.ethg$i <-- ethline <-- tic[3].gate$o++;
        node_11.ethg$o --> ethline --> tic[3].gate$i++;
        
        node_7.ethg$i <-- ethline <-- tic[3].gate$o++;
        node_7.ethg$o --> ethline --> tic[3].gate$i++;
        
        node_8.ethg$i <-- ethline <-- tic[3].gate$o++;
        node_8.ethg$o --> ethline --> tic[3].gate$i++;
        
        //to connect to the gateway 3
        node_10.ethg$i <-- ethline <-- tic[3].gate$o++;
        node_10.ethg$o --> ethline --> tic[3].gate$i++;
}