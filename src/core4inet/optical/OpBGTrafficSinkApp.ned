
package core4inet.optical;

import core4inet.applications.trafficsink.base.TrafficSinkApp;

//
// Traffic sink application used for statistics collection of best-effort traffic.
//
// @see ~TrafficSinkApp
//
// @author Till Steinbach
//
simple OpBGTrafficSinkApp extends TrafficSinkApp
{
    parameters:
        @class(OpBGTrafficSinkApp);
        @display("i=block/sink,green");

        //Source MAC address empty or 00:00:00:00:00:00 means all
        string srcAddress = default("00:00:00:00:00:00");      
        //Replay each frame;
        bool reply = default(false);
        //Replay first frame to update tables in switches;
        bool replyFirst = default(true);    

    gates:
        //Input gate for the incoming best-effort messages
        input In  @labels(EtherFrame);
}
