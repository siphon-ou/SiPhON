



cplusplus {{
//INET Auto-generated Messages
#include "inet/linklayer/ethernet/EtherFrame_m.h"

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined(__GNUC__) && __GNUC__ >= 5
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
}}

class inet::EthernetIIFrame;

namespace CoRE4INET;

//
// Ethernet II headers contain a 16-bit ~EtherType to identify
// the encapsulated protocol and 802.1Q Tag
//
// Header length: src(6)+dest(6)+etherType(2)+802.1QTag(4) = 18 byte
// Total overhead: 18 byte Header + 4 byte CRC = 22 byte
//
// @see ~EthernetIIFrame
//
// @author Philipp Meyer
//
packet OpticalDataPacket extends EthernetIIFrame
{
    simtime_t rate;
    simtime_t waitingTime;
    int streamID;
    simtime_t departureTime;
    simtime_t nextDepartureTime;
    simtime_t gatewayDepatureTime;
    simtime_t gatewayArrivalTime;
    simtime_t slotLeftMasternodeTime;
    int GatewayID;
    int bufferOccupancy1=-1;
    int bufferOccupancy2=-1;
    int bufferOccupancy3=-1;
    int64_t counter;
}
