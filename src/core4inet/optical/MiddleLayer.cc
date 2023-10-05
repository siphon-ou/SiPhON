
#include "MiddleLayer.h"


namespace CoRE4INET {

Define_Module(MiddleLayer);



void MiddleLayer::initialize()
{
    ApplicationBase::initialize();


}

void MiddleLayer::handleMessage(cMessage *msg)
{
    ApplicationBase::handleMessage(msg);

    if (msg->arrivedOn("AppIn"))
    {
        EV << "MiddleLayer::handleMessage msg->arrivedOn(AppIn) node "<<getIndex()<<" at "<<simTime()<<" \n";
        send(msg, "NetOut");
    }else if (msg->arrivedOn("NetIn"))
    {

        EV << "MiddleLayer::handleMessage msg->arrivedOn(NetIn) node "<<getIndex()<<" at "<<simTime()<<" \n";

        inet::EthernetIIFrame *msg2 = check_and_cast<inet::EthernetIIFrame *>(msg);
        std::string destination=msg2->getDest().str();


                        int app_destination=std::stoi (msg2->getDest().str().substr (15,2));


        send(msg, "AppOut",app_destination);
    }else
    {
        delete msg;
    }
}



}
