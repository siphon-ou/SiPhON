
//This app source decreases the packet send interval to 0.00003 during the simulation time between 1ms and 2ms

#include "OpBGTrafficSourceApp2.h"

//CoRE4INET
#include "core4inet/buffer/base/BGBuffer.h"
#include "core4inet/utilities/ConfigFunctions.h"
#include "OpticalDataPacket_m.h"

namespace CoRE4INET {

Define_Module(OpBGTrafficSourceApp2);

simsignal_t OpBGTrafficSourceApp2::sigSendInterval = registerSignal("intervalSignal");


OpBGTrafficSourceApp2::OpBGTrafficSourceApp2()
{
    this->sendInterval = 0;
    this->parametersInitialized = false;
    this->NextPacketDepatureIsKnown = false;
    this->streamID = -1;
}

void OpBGTrafficSourceApp2::initialize()
{
    TrafficSourceAppBase::initialize();
}

void OpBGTrafficSourceApp2::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        if (getEnvir()->isGUI())
        {
            getDisplayString().removeTag("i2");
        }
        EV << "OpBGTrafficSourceApp2::handleMessage at " << simTime() << "\n";

        if(simTime()<0.001){
            this->sendInterval=0.00012;
        }else if((simTime()>=0.001)&&(simTime()<0.002)){ //decrease the packet send interval to 0.00003 during the simulation time between 1ms and 2ms
            this->sendInterval=0.00003;
        }else if(simTime()>=0.002){
            this->sendInterval=0.00012;
        }

        sendMessage();
        scheduleAt(simTime() + this->sendInterval, msg);
        emit(sigSendInterval,this->sendInterval);
        handleParameterChange("sendInterval");
    }
    else
    {
        delete msg;
    }
}

void OpBGTrafficSourceApp2::sendMessage()
{

        inet::EthernetIIFrame *frame = new inet::EthernetIIFrame("Best-Effort Traffic", 7); //kind 7 = black

        frame->setDest(this->destAddress);

        cPacket *payload_packet = new cPacket();
        payload_packet->setByteLength(static_cast<int64_t>(getPayloadBytes()));
        frame->setByteLength(ETHER_MAC_FRAME_BYTES);
        frame->encapsulate(payload_packet);
        //Padding
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        {
            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
        }
        EV << "OpBGTrafficSourceApp2  send " << simTime() << "\n";


        OpticalDataPacket *msg = new OpticalDataPacket();
        msg->setRate(this->sendInterval);
        msg->setStreamID(this->streamID);
        msg->setDepartureTime(simTime());
        if(NextPacketDepatureIsKnown){

            msg->setNextDepartureTime(simTime() + this->sendInterval);
            EV << "OpBGTrafficSourceApp2 NextPacketDepatureIsKnown true. Send next at " << msg->getNextDepartureTime() <<" after " << msg->getNextDepartureTime()-simTime() <<" \n";
        }else{

            msg->setNextDepartureTime(0);
            //EV << "NextPacketDepatureIsKnown false " << msg->getNextDepartureTime() << "\n";
        }
        msg->setGatewayID(0);
        payload_packet->encapsulate(msg);




        send(frame,"Out");
        //sendDirect(frame, (*buf)->gate("in"));


}

void OpBGTrafficSourceApp2::handleParameterChange(const char* parname)
{
    TrafficSourceAppBase::handleParameterChange(parname);

    if (!parname && !parametersInitialized)
    {
        parametersInitialized = true;
    }

    if (!parname || !strcmp(parname, "sendInterval"))
    {
        this->sendInterval = parameterDoubleCheckRange(par("sendInterval"), 0, SIMTIME_MAX.dbl(), true);
    }
    if (!parname || !strcmp(parname, "destAddress"))
    {
        if (par("destAddress").stdstringValue() == "auto")
        {
            // assign automatic address
            this->destAddress = inet::MACAddress::generateAutoAddress();

            // change module parameter from "auto" to concrete address
            par("destAddress").setStringValue(this->destAddress.str());
        }
        else
        {
            this->destAddress.setAddress(par("destAddress").stringValue());
        }

    }

    if (!parname || !strcmp(parname, "streamID"))
    {
         streamID = par("streamID");
    }

    if (!parname || !strcmp(parname, "NextPacketDepatureIsKnown"))
        {
        NextPacketDepatureIsKnown = par("NextPacketDepatureIsKnown").boolValue();
        if(NextPacketDepatureIsKnown){
            EV << "NextPacketDepatureIsKnown is set true \n";
        }else{
            EV << "NextPacketDepatureIsKnown is set false \n";
            }

        }

}

inet::MACAddress OpBGTrafficSourceApp2::getDestAddress()
{
    if (!parametersInitialized)
    {
        handleParameterChange(nullptr);
    }
    return this->destAddress;
}

}
//namespace
