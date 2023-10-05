
package core4inet.optical;

import core4inet.applications.trafficsource.base.TrafficSourceAppBase;

//
// Background traffic source application.
// Uses EtherFrame with encapsulated cPacket as payload
// This generator generates frames with sendInterval distance
//
// @see ~TrafficSourceAppBase, ~TTTrafficSourceApp, ~RCTrafficSourceApp
//
// @author Till Steinbach
//
simple OpBGTrafficSourceApp extends TrafficSourceAppBase
{
    parameters:
        @class(OpBGTrafficSourceApp);
        @display("i=block/source,green");

        // destination MAC address
        string destAddress = default("auto");
        // interval between sending requests
        volatile double sendInterval @unit(s) = default(uniform(0s,1s));
        //id number
        int streamID;
        
        bool NextPacketDepatureIsKnown= default(false);
        
 
        
        @signal[intervalSignal];
        @statistic[sendIntervalOverTime](title="send interval over time"; source=intervalSignal; unit=s; record=stats?,histogram?,vector?; interpolationmode=linear);
        
     gates:
        input In;
        output Out;
}
