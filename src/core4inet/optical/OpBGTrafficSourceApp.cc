
#include "OpBGTrafficSourceApp.h"

//CoRE4INET
#include "core4inet/buffer/base/BGBuffer.h"
#include "core4inet/utilities/ConfigFunctions.h"
#include "OpticalDataPacket_m.h"

namespace CoRE4INET {

Define_Module(OpBGTrafficSourceApp);

simsignal_t OpBGTrafficSourceApp::sigSendInterval = registerSignal("intervalSignal");


OpBGTrafficSourceApp::OpBGTrafficSourceApp()
{
    this->sendInterval = 0;
    this->parametersInitialized = false;
    this->NextPacketDepatureIsKnown = false;
    this->streamID = -1;
}

void OpBGTrafficSourceApp::initialize()
{
    TrafficSourceAppBase::initialize();
}

void OpBGTrafficSourceApp::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        if (getEnvir()->isGUI())
        {
            getDisplayString().removeTag("i2");
        }
        EV << "OpBGTrafficSourceApp::handleMessage at " << simTime() << "\n";
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

void OpBGTrafficSourceApp::sendMessage()
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
        EV << "OpBGTrafficSourceApp  send " << simTime() << "\n";


        OpticalDataPacket *msg = new OpticalDataPacket();
        msg->setRate(this->sendInterval);
        msg->setStreamID(this->streamID);
        msg->setDepartureTime(simTime());
        if(NextPacketDepatureIsKnown){

            msg->setNextDepartureTime(simTime() + this->sendInterval);
            EV << "NextPacketDepatureIsKnown true. Send next at " << msg->getNextDepartureTime() <<" after " << msg->getNextDepartureTime()-simTime() <<" \n";
        }else{

            msg->setNextDepartureTime(0);
            //EV << "NextPacketDepatureIsKnown false " << msg->getNextDepartureTime() << "\n";
        }
        msg->setGatewayID(0);
        payload_packet->encapsulate(msg);




        send(frame,"Out");
        //sendDirect(frame, (*buf)->gate("in"));


}

void OpBGTrafficSourceApp::handleParameterChange(const char* parname)
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

inet::MACAddress OpBGTrafficSourceApp::getDestAddress()
{
    if (!parametersInitialized)
    {
        handleParameterChange(nullptr);
    }
    return this->destAddress;
}

}
//namespace
