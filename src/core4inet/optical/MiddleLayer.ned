
package core4inet.optical;

import core4inet.applications.AS6802.CTApplicationBase;
import core4inet.utilities.classes.IScheduled;

//
// Tic Application used for the rt_tictoc example.
// Sends TT messages with Tic-Request payload and receives
// RC messages with Toc-Response payload.
//
// @see rt_tictoc, ~TocApp
//
// @author Till Steinbach
//
simple MiddleLayer extends CTApplicationBase like IScheduled
{
   
    parameters:
        @class(MiddleLayer);
        @display("i=block/routing,cyan");
        
		//Signal emitted when a frame is received, contains received EtherFrame
        @signal[rxPk](type=inet::EtherFrame);
        //Signal emitted when a frame is received, contains roundtrip time
        @signal[roundtrip](type=simtime_t; unit=s;);
		//Statistic of the amount of packets that were received by the application
        @statistic[rxPk](title="Received Packets"; source=rxPk; unit=packets; record=count);
        //Statistic of the amount of bytes that were received by the application
        @statistic[rxBytes](title="Received Bytes"; source=rxPk; unit=B; record="sum(packetBytes)","vector(packetBytes)", "histogram(packetBytes)?"; interpolationmode=none);
        //Statistic of the applications roundtrip time
        @statistic[roundtrip](title="Roundtrip time"; unit=s; record=stats,histogram?,vector);
        


		//Oscillator of the timed module (must be set to the name of an oscillator module in the Node) when left empty: oscillator @see IScheduled
        string oscillator = default("^.scheduler.oscillator");
        //Timer of the timed module (must be set to the name of an timer module in the Node) when left empty: timer @see IScheduled
        string timer = default("^.scheduler.timer");
        //Period of the scheduled module (must be set to the name of a period module in the Node) when left empty: period[0] @see IScheduled
        string period = default("");
        
        
    gates:
        input AppIn[];
        output AppOut[];
        input NetIn;
        output NetOut;
}
