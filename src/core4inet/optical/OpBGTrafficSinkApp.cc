
#include "OpBGTrafficSinkApp.h"
#include "OpticalDataPacket_m.h"


//CoRE4INET
#include "core4inet/buffer/base/BGBuffer.h"

namespace CoRE4INET {

Define_Module(OpBGTrafficSinkApp);

OpBGTrafficSinkApp::OpBGTrafficSinkApp(){
    received = 0;
}

void OpBGTrafficSinkApp::initialize()
{
    TrafficSinkApp::initialize();

    handleParameterChange(nullptr);
}

void OpBGTrafficSinkApp::handleParameterChange(const char* parname)
{
    TrafficSinkApp::handleParameterChange(parname);
    if (!parname || !strcmp(parname, "srcAddress"))
    {
        if (par("srcAddress").stdstringValue() == "auto")
        {
            // change module parameter from "auto" to concrete address
            par("srcAddress").setStringValue(inet::MACAddress::UNSPECIFIED_ADDRESS.str());
        }
        address = inet::MACAddress(par("srcAddress").stringValue());
    }
}

void OpBGTrafficSinkApp::handleMessage(cMessage *msg)
{
    if (inet::EtherFrame *frame = dynamic_cast<inet::EtherFrame*>(msg))
    {
        //if (address.isUnspecified() || frame->getSrc() == address)
        //{


        CoRE4INET::OpticalDataPacket *msg4 = check_and_cast<CoRE4INET::OpticalDataPacket *>(frame->getEncapsulatedPacket()->getEncapsulatedPacket());

            EV << "1 OpBGTrafficSinkApp::handleMessage address.isUnspecified() "<<address.isUnspecified()<< " frame->getSrc() "<<frame->getSrc()<<" address "<<address
                    <<" DepartureTime "<<msg4->getDepartureTime()
                    <<" End-to-End Delay "<<simTime()-msg4->getDepartureTime()
                    <<" at "<< simTime() << "\n";
            double endtoenddelay=simTime().dbl()-msg4->getDepartureTime().dbl();
            double comparetime=7.0*pow(10,-7);
            if(endtoenddelay>comparetime){
                EV << "1 OpBGTrafficSinkApp::handleMessage Large end to end delay "<<simTime()-msg4->getDepartureTime()
                        <<" endtoenddelay "<<endtoenddelay
                        <<" compare "<<comparetime
                                <<" at "<< simTime() << "\n";
            }
            if(endtoenddelay<comparetime){
                EV << "1 OpBGTrafficSinkApp::handleMessage Small end to end delay "<<simTime()-msg4->getDepartureTime()
                <<" endtoenddelay "<<endtoenddelay
                <<" compare "<<comparetime
                                                    <<" at "<< simTime() << "\n";
                                }

                    received++;
            TrafficSinkApp::handleMessage(msg);
        //}
        //else
        //{
            //EV << "2 OpBGTrafficSinkApp::handleMessage address.isUnspecified() "<<address.isUnspecified()<< " frame->getSrc() "<<frame->getSrc()<<" address "<<address<<" at "<<frame->getSrc()<< simTime() << "\n";
            //delete msg;
        //}
    }
    else
    {
        EV << "3 OpBGTrafficSinkApp::handleMessage address.isUnspecified()  at "<<frame->getSrc()<< simTime() << "\n";
        delete msg;
    }
}

} //namespace
