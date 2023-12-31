//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package core4inet.linklayer.shaper.IEEE8021Qbv.selectionAlgorithm;

//
// Represents a Credit Based Shaper (CBS) implemenation for IEEE 802.1Qbv
//
// @see ~IEEE8021QbvSelectionAlgorithm
//
// @author Philipp Meyer
//
simple CreditBasedShaper extends IEEE8021QbvSelectionAlgorithm
{
    parameters:
    	@class(CreditBasedShaper);
    	// Signal emitted when the credit of the CBS algorithm changes
    	//Onur
        @signal[credit](type=double);
        // Statistic of the credit of the CBS algorithm
        @statistic[credit](title="Credit"; source=credit; unit=b; record=vector; interpolationmode=linear;);   	
    	// Stream Reservation Class of the Shaper
        string srClass = default("A");
    	// Oscillator of the timed module (must be set to the name of an oscillator module in the Node) when left empty: oscillator
        string oscillator = default("^.^.^.scheduler.oscillator");
        // Timer of the timed module (must be set to the name of an timer module in the Node) when left empty: timer
        string timer = default("^.^.^.scheduler.timer");
        
     gates:
         //Input gate for the incoming SchedulerActionTimeEvent messages
         input schedulerIn @directIn;
}
