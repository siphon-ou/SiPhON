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
package core4inet.buffer.AS6802;

//
// Represents a queue buffer for time-triggered traffic. Messages are sent using the configured
// send window.
//
// The buffer stores "size" messages in a fifo queue.
// The buffer removes and sends the first message in the queue.
// The when full the oldest frame is dropped before inserting the new frame. Can be configured to drop new frame instead
// using the dropNew parameter (default: false)
//
// @see TTBuffer, Buffer,
//
// @author Till Steinbach
simple TTQueueBuffer extends TTBuffer
{
    parameters:
        //TTQueueBuffer is implemented by the TTQueueBuffer C++ Class
        @class(TTQueueBuffer);
        @display("q=frames");
        //Signal emitted when queuelength changes, contains new length
        @signal[length](type=unsigned long; unit=packets);
        //Signal emitted when queuelength changes, contains new size in bytes
        @signal[size](type=unsigned long; unit=B);
        //Signal emitted when frame is dropped due to buffer overflow, contains dropped EtherFrame
        @signal[dropped](type=inet::EtherFrame);
        
        //Statistic of the queue length of the buffer
        @statistic[length](title="Queue Length"; source=length; unit=packets; record=vector,stats; interpolationmode=sample-hold);
        //Statistic of the queue size of the buffer in bytes
        @statistic[size](title="Queue Size"; source=size; record=vector,stats; unit=B; interpolationmode=sample-hold);
        //Statistic of the amount of packets that were dropped due to buffer overflows
        @statistic[dropped](title="Dropped"; source=dropped; unit=packets; record=count);
        
        //Size of the buffer negative means infinite
        int size = default(-1);
        //Dropping policy, Should the newest frame instead of the oldest frame be dropped when the buffer is full (see size parameter)
        bool dropNew = default(false);
}
