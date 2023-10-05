#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "OpticalControlPacket_m.h"
#include <omnetpp/cqueue.h>

using namespace omnetpp;

class EndHost : public cSimpleModule
{

    ~EndHost();
private:
    simsignal_t arrivalSignal;

    simtime_t timerInterval;
    simtime_t controlPacketDuration;
    simtime_t offset;



    double datapacketsize;
    double guardsize;
    double controlpacketsize;
    double DataWaveBandwidth;
    double ControlWaveBandwidth;


    cQueue *ToOpticalRingQueue;

protected:
    virtual OpticalControlPacket *generateMessage();
    virtual void forwardMessage(OpticalControlPacket *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(EndHost);

void EndHost::initialize()
{
    arrivalSignal = registerSignal("arrival");
    guardsize=20.0;
    datapacketsize=1522.0+guardsize;
    controlpacketsize=50.0;
    DataWaveBandwidth=  100000000000.0;
    ControlWaveBandwidth=12500000000.0;
    timerInterval = (datapacketsize*8.0)/DataWaveBandwidth; // transfer time of one data packet
    controlPacketDuration=(controlpacketsize*8.0)/ControlWaveBandwidth;
    offset=0.00003;


}

void EndHost::handleMessage(cMessage *msg)
{

}

OpticalControlPacket *EndHost::generateMessage()
{
    // Produce source and destination addresses.
    int src = getIndex();
    int n = getVectorSize();
    int dest = intuniform(0, n-2);
    if (dest >= src)
        dest++;

    char msgname[20];
    sprintf(msgname, "tic-%d-to-%d", src, dest);

    // Create message object and set source and destination field.
    OpticalControlPacket *msg = new OpticalControlPacket(msgname);
    msg->setSource(src);
    msg->setDestination(-1);
    msg->setType(-1);
    return msg;
}

void EndHost::forwardMessage(OpticalControlPacket *msg)
{
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);

    // Same routing as before: random gate.
    int n = gateSize("gate");
    int k = intuniform(0, n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}


EndHost::~EndHost()
{
    // remove messages
    cMessage *msg;
    while(!ToOpticalRingQueue->isEmpty()) {
        msg = (cMessage *)ToOpticalRingQueue->pop();
        delete msg;
    }
    delete ToOpticalRingQueue;
}
