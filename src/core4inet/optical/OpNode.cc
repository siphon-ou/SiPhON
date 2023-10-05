#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "OpticalControlPacket_m.h"
#include "OpticalDataPacket_m.h"
#include <omnetpp/cqueue.h>
#include "inet/linklayer/ppp/PPP.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/common/MACAddress.h"

using namespace omnetpp;

class OpNode : public cSimpleModule
{

    ~OpNode();
private:
    simsignal_t arrivalSignal;

    simtime_t timerInterval;
    simtime_t controlPacketDuration;
    simtime_t offset;
    simtime_t video_packet_period;
    simtime_t first_control_time;
    cMessage * control_packet_generator_timer;
    cMessage * send_data_buffer;


    double datapacketsize;
    double controlpacketsize;
    double DataWaveBandwidth;
    double ControlWaveBandwidth;

    int currentslot;
    int slotcount;
    int channelallocationtype;
    int gatewaycount;
    int received_first_control;

    int64_t counter; //slot counter
    int64_t last_received_talk_control_for_this_gateway;

    //int schedulelist[10];
    int schedulelist[8];


    //buffer stats of control packets of ECUs
    int bufferstats1[20];

    //buffer stats of data packets of ECUs
    int bufferstats2[20];
    int bufferstats3[20];

    int SlotAssignmentList[10000000]; //reservation list for 10 million slots. Later update it to dynamic list.
    int SlotAssignmentPriorityList[10000000]; //reservation priority list for 10 million slots. Later update it to dynamic list.

    //the last time a buffer was selected
    simtime_t lastselected[20];

    simtime_t *SendFinishTime;

    simtime_t BackboneDelay; //time spent for a loop of data slot
    simtime_t FirstSlotDepartureTime;



    cQueue *ToOpticalRingQueue; //control packets
    cQueue *ToOpticalRingQueue2; //high priority data packets
    cQueue *ToOpticalRingQueue3; //low priority data packets

protected:
    virtual OpticalControlPacket *generateMessage(int64_t counter);
    virtual void forwardMessage(OpticalControlPacket *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(OpNode);

void OpNode::initialize()
{

    arrivalSignal = registerSignal("arrival");
    datapacketsize=par("slotsize"); //used to be 1522.0+guardsize
    channelallocationtype=par("channelallocationtype");
    gatewaycount=par("gatewaycount");
    controlpacketsize=20.0;
    DataWaveBandwidth=  100000000000.0;
    ControlWaveBandwidth= 1250000000.0;
    video_packet_period=par("video_packet_period");
    offset=par("guard"); //50nsec 0.00000005s
    timerInterval = offset+((datapacketsize*8.0)/DataWaveBandwidth); // offset (guard band) plus transfer time of one data slot
    controlPacketDuration=(controlpacketsize*8.0)/ControlWaveBandwidth;



    currentslot=0;
    BackboneDelay=0;
    slotcount=7; //old papers use this
    //slotcount=8;
    FirstSlotDepartureTime=0;
    SendFinishTime = new simtime_t[gateSize("gate")];
    received_first_control=0;
    first_control_time=0;
    counter=0;
    last_received_talk_control_for_this_gateway=0;


    for (int i=0; i<gateSize("gate"); i++) {
        SendFinishTime[i] = -1;    // Initialize all elements to zero.
    }

    //index 0 is for master node
    for (int i=0; i<gatewaycount+1; i++) {
        bufferstats1[i] = 0;    // Initialize all elements to zero.
        bufferstats2[i] = 0;    // Initialize all elements to zero.
        bufferstats3[i] = 0;    // Initialize all elements to zero.
        lastselected[i] = 0;    // Initialize all elements to zero.
    }

    for (int i=0; i<10000000; i++) {
        SlotAssignmentList[i] = -1;    // Initialize all elements to -1.
        SlotAssignmentPriorityList[i] = 100;    // Initialize all elements to 100.
    }



    //    schedulelist[0]=0;
    //    schedulelist[1]=1;
    //    schedulelist[2]=0;
    //    schedulelist[3]=2;
    //    schedulelist[4]=0;
    //    schedulelist[5]=3;
    //    schedulelist[6]=0;
    //    schedulelist[7]=4;
    //    schedulelist[8]=0;
    //    schedulelist[9]=5;

    /*    schedulelist[0]=0;
    schedulelist[1]=1;
    schedulelist[2]=2;
    schedulelist[3]=3;
    schedulelist[4]=3;
    schedulelist[5]=3;
    schedulelist[6]=4;
    schedulelist[7]=5;*/


    /*    schedulelist[0]=0;
    schedulelist[1]=1;
    schedulelist[2]=3;
    schedulelist[3]=2;
    schedulelist[4]=4;
    schedulelist[5]=3;
    schedulelist[6]=5;
    schedulelist[7]=3;*/


    //old papers use this
    schedulelist[0]=0;
    schedulelist[1]=1;
    schedulelist[2]=2;
    schedulelist[3]=3;
    schedulelist[4]=4;
    schedulelist[5]=5;
    schedulelist[6]=3;

    /*
    schedulelist[0]=0;
    schedulelist[1]=1;
    schedulelist[2]=3;
    schedulelist[3]=2;
    schedulelist[4]=4;
    schedulelist[5]=3;
    schedulelist[6]=5;
    schedulelist[7]=3;
     */

    send_data_buffer = new cMessage("send data to optical ring");




    ToOpticalRingQueue = new cQueue("ToOpticalRing");
    ToOpticalRingQueue2 = new cQueue("ToOpticalRing2");
    ToOpticalRingQueue3 = new cQueue("ToOpticalRing3");

    // Node 0 sends the first message
    if (getIndex() == 0) {

        control_packet_generator_timer = new cMessage("control packet generator timer");
        scheduleAt(simTime() + timerInterval, control_packet_generator_timer);
        EV << "Rescheduled tick to " << simTime() + timerInterval <<" with timerinterval "<<timerInterval<< " at " << simTime() << "\n";

        // The initial message is sent based on tail departure at simTime() + controlPacketDuration.


        OpticalControlPacket *msg = generateMessage(counter);
        counter++;
        msg->setType(-1);
        msg->setDestination(-1);
        sendDelayed(msg,controlPacketDuration,"control_out");
        currentslot=(currentslot+1)%slotcount;
        EV << "currentslot changed to " << currentslot <<" for schedulelist[currentslot] "<<schedulelist[currentslot]<<" at " << simTime() << "\n";

        EV << "The initial control message is sent based on tail departure at " << simTime() + controlPacketDuration << " at " << simTime() << "\n";
        //scheduleAt(simTime() + controlPacketDuration, msg);

        if(FirstSlotDepartureTime==0){
            FirstSlotDepartureTime=simTime();
            EV << "FirstSlotDepartureTime is set to simTime() " << simTime() << "\n";
        }

    }
}

void OpNode::handleMessage(cMessage *msg)
{
    //EV << "datapacketsize "<<datapacketsize<< "\n";


    if (msg == control_packet_generator_timer) {
        //timer for generating control packet at master node




        scheduleAt(simTime() + timerInterval, control_packet_generator_timer);  // rescheduling timer
        EV << "Rescheduled tick to " << simTime() + timerInterval <<" with timerinterval "<<timerInterval<< " at " << simTime() << "\n";


        //control packet will finish departing the node after (controlpacketsize*8.0)/(12500000000.0)
        EV << "channelallocationtype " << channelallocationtype << " at " << simTime() << "\n";




        if(channelallocationtype==0){
            //periodic

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();
            bufferstats3[0]=ToOpticalRingQueue3->getLength();

            //index 0 is for master node
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " bufferstats3[i] "<<bufferstats3[i]<< " at " << simTime() << "\n";
            }

            if(schedulelist[currentslot]<gatewaycount+1){
                //bufferstats[schedulelist[currentslot]]--;
            }else{
                EV << "ERROR bufferstats out of bounds schedulelist[currentslot] "<<schedulelist[currentslot]<< " at " << simTime() << "\n";
            }

        }else if(channelallocationtype==1){
            // random

            bufferstats1[0]=ToOpticalRingQueue->getLength()+ToOpticalRingQueue2->getLength()+ToOpticalRingQueue3->getLength();

            int nonemptycount=0;
            int nonemptylist[gatewaycount];
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " bufferstats3[i] "<<bufferstats3[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]+bufferstats2[i]+bufferstats3[i]>0){
                    nonemptylist[nonemptycount]=i;
                    nonemptycount++;
                }
            }
            int selected=0;
            int randomindex=-1;
            if(nonemptycount>0){
                //there are one or more gateway/master with packets to send in their buffers
                randomindex=intuniform(0,nonemptycount-1);
                selected=nonemptylist[randomindex];
            }else{
                //it looks like all buffers are empty.we know that master node's buffer is empty too, so choose a random gateway to send a listen slot
                //This allows more sampling of buffer occupancy of gateways. Also the selected gateway may have a packet in its buffer after it was last sampled.
                //1 is added to random number to select a gateway, because index 0 is for master node
                selected=intuniform(0,gatewaycount-1)+1;
            }

            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount "<<nonemptycount<<" randomindex "<<randomindex<<" at " << simTime() << "\n";


        }else if(channelallocationtype==2){
            // control packet priority + max occupied buffer priority

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;

            if(nonemptycount1>0){
                //there are one or more gateway/master with control packets to send in their buffers
                selected=maxindex1;
                EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
            }else if(nonemptycount2>0){
                //there are one or more gateway/master with data packets to send in their buffers
                selected=maxindex2;
            }else{
                //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                selected=1;
                simtime_t latesttime=simTime();
                for (int i=1; i<gatewaycount+1; i++) {
                    EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                    if(lastselected[i]<latesttime){
                        selected=i;
                        latesttime=lastselected[i];
                        EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==3){
            // control packet priority + max occupied buffer priority + assign idle slots to the gateway which was selected at the oldest time.

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;

            if(nonemptycount1>0){
                //there are one or more gateway/master with control packets to send in their buffers
                selected=maxindex1;
                EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
            }else if(nonemptycount2>0){
                //there are one or more gateway/master with data packets to send in their buffers
                selected=maxindex2;
            }else{
                //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                selected=1;
                simtime_t latesttime=simTime();
                for (int i=1; i<gatewaycount+1; i++) {
                    EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                    if(lastselected[i]<latesttime){
                        selected=i;
                        latesttime=lastselected[i];
                        EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==4){
            // traffic information + control packet priority + max occupied buffer priority for only master gateway

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<<" channelallocationtype "<<channelallocationtype<<" video_packet_period "<<video_packet_period<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;




            for (int i=1; i<gatewaycount+1; i++) {

                simtime_t arrival=video_packet_period;

                if(i==3){
                    arrival=arrival/2;
                }
                if(lastselected[i]<simTime()-arrival){
                    selected=i;

                    EV << "Based on traffic information arrival "<<arrival <<" i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" at " << simTime() << "\n";
                    i=gatewaycount+1;
                }


            }
            if(selected==0){
                if(nonemptycount1>0){
                    //there are one or more gateway/master with control packets to send in their buffers
                    selected=maxindex1;
                    EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
                }else if(nonemptycount2>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex2;
                }else{
                    //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                    selected=1;
                    simtime_t latesttime=simTime();
                    for (int i=1; i<gatewaycount+1; i++) {
                        EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        if(lastselected[i]<latesttime){
                            selected=i;
                            latesttime=lastselected[i];
                            EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        }
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==5){
            // traffic information (exactly the same period scheduling) + control packet priority + max occupied buffer priority for only master gateway

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;




            for (int i=1; i<gatewaycount+1; i++) {

                simtime_t arrival=video_packet_period;

                if(i==3){
                    arrival=arrival/2;
                }
                if(lastselected[i]<simTime()-arrival){
                    selected=i;

                    EV << "Based on traffic information arrival "<<arrival <<" i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" at " << simTime() << "\n";
                    i=gatewaycount+1;
                }


            }
            if(selected==0){
                if(nonemptycount1>0){
                    //there are one or more gateway/master with control packets to send in their buffers
                    selected=maxindex1;
                    lastselected[selected]=simTime();
                    EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
                }else if(nonemptycount2>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex2;
                    lastselected[selected]=simTime();
                }else{
                    //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                    selected=1;
                    simtime_t latesttime=simTime();
                    for (int i=1; i<gatewaycount+1; i++) {
                        EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        if(lastselected[i]<latesttime){
                            selected=i;
                            latesttime=lastselected[i];
                            EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        }
                    }

                }
            }


            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==6){
            // traffic information + control packet priority + max occupied buffer priority

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;




            for (int i=1; i<gatewaycount+1; i++) {

                simtime_t arrival=video_packet_period;

                if(i==3){
                    arrival=arrival/2;
                }
                if(lastselected[i]<simTime()-arrival){
                    selected=i;

                    EV << "Based on traffic information arrival "<<arrival <<" i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" at " << simTime() << "\n";
                    i=gatewaycount+1;
                }


            }
            if(selected==0){
                if(nonemptycount1>0){
                    //there are one or more gateway/master with control packets to send in their buffers
                    selected=maxindex1;
                    EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
                }else if(nonemptycount2>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex2;
                }else{
                    //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                    selected=1;
                    simtime_t latesttime=simTime();
                    for (int i=1; i<gatewaycount+1; i++) {
                        EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        if(lastselected[i]<latesttime){
                            selected=i;
                            latesttime=lastselected[i];
                            EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        }
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==7){
            // traffic information + control packet priority + max occupied buffer priority+assign idle slots to the gateway which was selected at the oldest time.

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
            }
            int selected=0;




            for (int i=1; i<gatewaycount+1; i++) {

                simtime_t arrival=video_packet_period;

                if(i==3){
                    arrival=arrival/2;
                }
                if(lastselected[i]<simTime()-arrival){
                    selected=i;

                    EV << "Based on traffic information arrival "<<arrival <<" i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" at " << simTime() << "\n";
                    i=gatewaycount+1;
                }


            }
            if(selected==0){
                if(nonemptycount1>0){
                    //there are one or more gateway/master with control packets to send in their buffers
                    selected=maxindex1;
                    EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
                }else if(nonemptycount2>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex2;
                }else{
                    //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                    selected=1;
                    simtime_t latesttime=simTime();
                    for (int i=1; i<gatewaycount+1; i++) {
                        EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        if(lastselected[i]<latesttime){
                            selected=i;
                            latesttime=lastselected[i];
                            EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        }
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else if(channelallocationtype==8){
            // traffic information and arrival/departure time calculation

            bufferstats1[0]=ToOpticalRingQueue->getLength();
            bufferstats2[0]=ToOpticalRingQueue2->getLength();
            bufferstats3[0]=ToOpticalRingQueue3->getLength();

            int nonemptycount1=0;
            int nonemptycount2=0;
            int nonemptycount3=0;
            int maxbuffer1=0;
            int maxindex1=0;
            int maxbuffer2=0;
            int maxindex2=0;
            int maxbuffer3=0;
            int maxindex3=0;
            for (int i=0; i<gatewaycount+1; i++) {
                EV << "bufferstats i " <<i <<" bufferstats1[i] "<<bufferstats1[i]<< " bufferstats2[i] "<<bufferstats2[i]<< " bufferstats3[i] "<<bufferstats3[i]<< " at " << simTime() << "\n";

                if(bufferstats1[i]>maxbuffer1){
                    maxbuffer1=bufferstats1[i];
                    maxindex1=i;
                    nonemptycount1++;
                }
                if(bufferstats2[i]>maxbuffer2){
                    maxbuffer2=bufferstats2[i];
                    maxindex2=i;
                    nonemptycount2++;
                }
                if(bufferstats3[i]>maxbuffer3){
                    maxbuffer3=bufferstats3[i];
                    maxindex3=i;
                    nonemptycount3++;
                }
            }
            int selected=0;



            EV <<"Current slot "<<simTime()/timerInterval<<" at " << simTime() << "\n";
            int64_t overallcurrentslot=(simTime())/timerInterval;

            //check if there is already a slot reservation
            if(SlotAssignmentList[overallcurrentslot]>-1){

                selected=SlotAssignmentList[overallcurrentslot];
                EV <<"Send scheduled slot to gateway "<<selected<<" at " << simTime() << "\n";
            }

            if(selected==0){
                //there is no slot reservation. Check if there is a master or gateway that has packets in its buffers to send
                if(nonemptycount1>0){
                    //there are one or more gateway/master with control packets to send in their buffers
                    selected=maxindex1;
                    EV << "selected based on control packets. Selected "<<selected<<" at " << simTime() << "\n";
                }else if(nonemptycount2>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex2;
                }else if(nonemptycount3>0){
                    //there are one or more gateway/master with data packets to send in their buffers
                    selected=maxindex3;
                }else{
                    //it looks like all buffers are empty. Select the gateway which was selected at the oldest time.
                    selected=1;
                    simtime_t latesttime=simTime();
                    for (int i=1; i<gatewaycount+1; i++) {
                        EV << "all buffers are empty i "<<i<<" current selected " <<selected<< " lastselected[i] "<<lastselected[i]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        if(lastselected[i]<latesttime){
                            selected=i;
                            latesttime=lastselected[i];
                            EV << "updated all buffers are empty i "<<i<<" current selected" <<selected<< " lastselected[selected] "<<lastselected[selected]<<" latesttime "<<latesttime<<" at " << simTime() << "\n";
                        }
                    }
                }
            }

            lastselected[selected]=simTime();
            schedulelist[currentslot]=selected;

            EV << "selected " <<selected<< " nonemptycount1 "<<nonemptycount1<<" nonemptycount2 "<<nonemptycount2<<" nonemptycount3 "<<nonemptycount3<<" lastselected[selected] "<<lastselected[selected]<<" at " << simTime() << "\n";


        }else{

        }

        if(schedulelist[currentslot]==0){
            //master node is selected for sending packet to a gateway
            EV << "control_packet_generator_timer schedulelist[currentslot]==0 at " << simTime() << "\n";


            if(!ToOpticalRingQueue->isEmpty()) {
                //send the data packet in the high priority queue to the optical ring
                EV << "Master Opnode will send DATA packet based on tail departure at " << controlPacketDuration+offset << " at " << simTime() << "\n";
                cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue->pop();

                inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
                //std::string destination=msgdata->getDest().str();

                int core_destination=std::stoi (msgdata->getDest().str().substr (9,2));




                EV << "Master Opnode will send LISTEN control packet based on tail departure at " << controlPacketDuration << " at " << simTime() << "\n";
                //generate a new control packet with the data packet info
                OpticalControlPacket *msg = generateMessage(counter);
                counter++;
                msg->setType(0);
                msg->setDestination(core_destination);//index starts from 1. 1 means first gateway

                sendDelayed(msgToOpticalRingQueue,controlPacketDuration+offset,"data_out",0);
                sendDelayed(msg,controlPacketDuration,"control_out");

            }else if(!ToOpticalRingQueue2->isEmpty()) {
                //send the data packet in the middle priority queue to the optical ring
                EV << "Master Opnode will send DATA packet based on tail departure at " << controlPacketDuration+offset << " at " << simTime() << "\n";
                cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue2->pop();

                inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
                //std::string destination=msgdata->getDest().str();

                int core_destination=std::stoi (msgdata->getDest().str().substr (9,2));




                EV << "Master Opnode will send LISTEN control packet based on tail departure at " << controlPacketDuration << " at " << simTime() << "\n";
                //generate a new control packet with the data packet info
                OpticalControlPacket *msg = generateMessage(counter);
                counter++;
                msg->setType(0);
                msg->setDestination(core_destination);//index starts from 1. 1 means first gateway

                sendDelayed(msgToOpticalRingQueue,controlPacketDuration+offset,"data_out",0);
                sendDelayed(msg,controlPacketDuration,"control_out");

            }else if(!ToOpticalRingQueue3->isEmpty()) {
                //send the data packet in the middle priority queue to the optical ring
                EV << "Master Opnode will send DATA packet based on tail departure at " << controlPacketDuration+offset << " at " << simTime() << "\n";
                cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue3->pop();

                inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
                //std::string destination=msgdata->getDest().str();

                int core_destination=std::stoi (msgdata->getDest().str().substr (9,3));




                EV << "Master Opnode will send LISTEN control packet based on tail departure at " << controlPacketDuration << " at " << simTime() << "\n";
                //generate a new control packet with the data packet info
                OpticalControlPacket *msg = generateMessage(counter);
                counter++;
                msg->setType(0);
                msg->setDestination(core_destination);//index starts from 1. 1 means first gateway

                sendDelayed(msgToOpticalRingQueue,controlPacketDuration+offset,"data_out",0);
                sendDelayed(msg,controlPacketDuration,"control_out");

            }else{

                EV << "Error. EMPTY is not used currently. at " << simTime() << "\n";


                EV << "Master Opnode will send EMPTY control packet based on tail departure at " << controlPacketDuration << " at " << simTime() << "\n";
                //generate a new control packet with the data packet info
                OpticalControlPacket *msg = generateMessage(counter);
                counter++;
                msg->setType(-1);
                msg->setDestination(-1);
                sendDelayed(msg,controlPacketDuration,"control_out");
            }
        }else{
            EV << "Master Opnode will send TALK control packet based on tail departure at " << controlPacketDuration << " at " << simTime() << "\n";
            //generate a new control packet with the data packet info
            OpticalControlPacket *msg = generateMessage(counter);
            counter++;
            msg->setType(1);
            EV << "Master Opnode setDestination "<<schedulelist[currentslot]<<" currentslot "<<currentslot<<" at " << controlPacketDuration << " at " << simTime() << "\n";
            msg->setDestination(schedulelist[currentslot]);
            sendDelayed(msg,controlPacketDuration,"control_out");
        }


        currentslot=(currentslot+1)%slotcount;
        EV << "currentslot changed to " << currentslot <<" for schedulelist[currentslot] "<<schedulelist[currentslot]<<" at " << simTime() << "\n";

        EV << "The next control message is sent based on tail departure at " << simTime() + controlPacketDuration << " at " << simTime() << "\n";
        //scheduleAt(simTime() + controlPacketDuration, msg);

    }else if(msg == send_data_buffer){
        //send from data buffer if a packet is available
        //used by the slave gateways



        if(0 == getIndex()){
            EV << "ERROR: should not enter msg == send_data_buffer if(0 == getIndex()) at " << simTime() << "\n";
        }

        if(!ToOpticalRingQueue->isEmpty()) {
            //send the data packet in the high priority queue to the optical ring
            EV << "Gateway Opnode id "<<getIndex()<<" will start sending the head of data packet in slot "<<last_received_talk_control_for_this_gateway<<" based on tail departure time " << (datapacketsize*8.0)/DataWaveBandwidth << " at " << simTime() << "\n";
            cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue->pop();

            inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
            std::string destination=msgdata->getDest().str();



            CoRE4INET::OpticalDataPacket *msg4 = check_and_cast<CoRE4INET::OpticalDataPacket *>(msgdata->getEncapsulatedPacket()->getEncapsulatedPacket());
            msg4->setWaitingTime(simTime()-msg4->getWaitingTime());//put the waiting time information of the packet in the gateway
            msg4->setGatewayDepatureTime(simTime());//put the head departure time information of the packet in the gateway
            msg4->setCounter(last_received_talk_control_for_this_gateway);//slot no

            msg4->setBufferOccupancy1(ToOpticalRingQueue->getLength());
            msg4->setBufferOccupancy2(ToOpticalRingQueue2->getLength());
            msg4->setBufferOccupancy3(ToOpticalRingQueue3->getLength());

            sendDelayed(msgToOpticalRingQueue,(datapacketsize*8.0)/DataWaveBandwidth,"data_out",0);
        }else if(!ToOpticalRingQueue2->isEmpty()) {
            //send the data packet in the second priority queue to the optical ring
            EV << "Gateway Opnode id "<<getIndex()<<" will start sending the head of data packet in a slot" << (datapacketsize*8.0)/DataWaveBandwidth << " at " << simTime() << "\n";
            cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue2->pop();

            inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
            std::string destination=msgdata->getDest().str();



            CoRE4INET::OpticalDataPacket *msg4 = check_and_cast<CoRE4INET::OpticalDataPacket *>(msgdata->getEncapsulatedPacket()->getEncapsulatedPacket());
            msg4->setWaitingTime(simTime()-msg4->getWaitingTime());//put the waiting time information of the packet in the gateway
            msg4->setGatewayDepatureTime(simTime());//put the head departure time information of the packet in the gateway
            msg4->setCounter(last_received_talk_control_for_this_gateway);//slot no


            msg4->setBufferOccupancy1(ToOpticalRingQueue->getLength());
            msg4->setBufferOccupancy2(ToOpticalRingQueue2->getLength());
            msg4->setBufferOccupancy3(ToOpticalRingQueue3->getLength());

            if((!msgdata->getDest().str().compare("0A-10-00-00-00-05"))||(!msgdata->getDest().str().compare("0A-10-00-00-00-04"))){
                EV << "Message with ID "<<msgToOpticalRingQueue->getId()<<" send_data_buffer(beginning of slot) start sending to ring for destination "<<msgdata->getDest().str()<<" sent from " << getSimulation()->getModule(msgToOpticalRingQueue->getSenderModuleId())->getFullPath() << " has latency "<<simTime()-msgToOpticalRingQueue->getCreationTime()<< " at " << simTime() << " and was created " << msgToOpticalRingQueue->getCreationTime() << "\n";
            }

            sendDelayed(msgToOpticalRingQueue,(datapacketsize*8.0)/DataWaveBandwidth,"data_out",0);
        }else if(!ToOpticalRingQueue3->isEmpty()) {
            //send the data packet in the low priority queue to the optical ring
            EV << "Gateway Opnode id "<<getIndex()<<" will start sending the head of data packet in a slot " << (datapacketsize*8.0)/DataWaveBandwidth << " at " << simTime() << "\n";
            cMessage *msgToOpticalRingQueue= (cMessage *)ToOpticalRingQueue3->pop();

            inet::EthernetIIFrame *msgdata = check_and_cast<inet::EthernetIIFrame *>(msgToOpticalRingQueue);
            std::string destination=msgdata->getDest().str();



            CoRE4INET::OpticalDataPacket *msg4 = check_and_cast<CoRE4INET::OpticalDataPacket *>(msgdata->getEncapsulatedPacket()->getEncapsulatedPacket());
            msg4->setWaitingTime(simTime()-msg4->getWaitingTime());//put the waiting time information of the packet in the gateway
            msg4->setGatewayDepatureTime(simTime());//put the head departure time information of the packet in the gateway
            msg4->setCounter(last_received_talk_control_for_this_gateway);//slot no


            msg4->setBufferOccupancy1(ToOpticalRingQueue->getLength());
            msg4->setBufferOccupancy2(ToOpticalRingQueue2->getLength());
            msg4->setBufferOccupancy3(ToOpticalRingQueue3->getLength());

            if((!msgdata->getDest().str().compare("0A-10-00-00-00-05"))||(!msgdata->getDest().str().compare("0A-10-00-00-00-04"))){
                EV << "Message with ID "<<msgToOpticalRingQueue->getId()<<" send_data_buffer(beginning of slot) start sending to ring for destination "<<msgdata->getDest().str()<<" sent from " << getSimulation()->getModule(msgToOpticalRingQueue->getSenderModuleId())->getFullPath() << " has latency "<<simTime()-msgToOpticalRingQueue->getCreationTime()<< " at " << simTime() << " and was created " << msgToOpticalRingQueue->getCreationTime() << "\n";
            }

            sendDelayed(msgToOpticalRingQueue,(datapacketsize*8.0)/DataWaveBandwidth,"data_out",0);
        }else{
            //no packet to send in the buffers. Just send the statistics in a slot. Create a slot.
            inet::EthernetIIFrame *frame = new inet::EthernetIIFrame("E", 7); //kind 7 = black


            cPacket *payload_packet = new cPacket();
            //              payload_packet->setByteLength(static_cast<int64_t>(getPayloadBytes()));
            frame->setByteLength(ETHER_MAC_FRAME_BYTES);
            frame->encapsulate(payload_packet);
            //Padding
            if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            {
                frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
            }
            EV << "Gateway Opnode id "<<getIndex()<<" has no packet to send in the buffers. Just send the statistics in a slot. Create a slot at " << simTime() << "\n";


            CoRE4INET::OpticalDataPacket *msg = new CoRE4INET::OpticalDataPacket();
            msg->setDepartureTime(simTime());
            msg->setNextDepartureTime(0);
            msg->setGatewayID(getIndex());
            msg->setStreamID(-1);//-1 shows that this slot is empty
            msg->setCounter(last_received_talk_control_for_this_gateway);//slot no

            msg->setBufferOccupancy1(ToOpticalRingQueue->getLength());
            msg->setBufferOccupancy2(ToOpticalRingQueue2->getLength());
            msg->setBufferOccupancy3(ToOpticalRingQueue3->getLength());


            payload_packet->encapsulate(msg);

            sendDelayed(frame,(datapacketsize*8.0)/DataWaveBandwidth,"data_out",0);



        }


    }else if (msg->isSelfMessage()){


        send(msg, "control_out");
        std::cout << "ERROR: should not enter msg->isSelfMessage() at " << simTime() << "\n";

    }else if (msg->arrivedOn("control_in")){
        //received the tail of a control message from optical ring

        if (0 == getIndex()){
            //Control packet arrived back to master node. Packet is no longer needed so delete it.
            delete msg;

            if((SIMTIME_DBL(BackboneDelay)==0)){
                BackboneDelay=simTime()-FirstSlotDepartureTime-controlPacketDuration;
                EV << "BackboneDelay is "<<BackboneDelay<<" simTime() "<< simTime()<<" FirstSlotDepartureTime "<<FirstSlotDepartureTime<<" controlPacketDuration "<<controlPacketDuration  << "\n";
            }

            EV << "(0 == getIndex())  the tail of control packet arrived back to master node at " << simTime() << "\n";
        }else{
            //finished receiving a control packet from adjacent node.
            //process a copy and send the original to next node without delay.
            OpticalControlPacket *ttmsg = check_and_cast<OpticalControlPacket *>(msg);

            EV << "GW "<<getIndex()<<" finished receiving the tail of a control packet for slot number "<<ttmsg->getCounter()<<" for "<<ttmsg->getDestination()<<" at " << simTime() << "\n";

            //store the arrival time of the first control message to calculate slot arrival times
            if(received_first_control==0){
                first_control_time=simTime();
                received_first_control=1;
                EV << "Opnode id "<<getIndex()<<" first_control_time " << first_control_time << "\n";
            }

            if (ttmsg->getDestination() == getIndex()) {//control packet is for this gateway

                if (0 == ttmsg->getType()) {
                    //listen message is received. The gateway will listen and receive a packet to from master after offset time

                    //listen packets cannot carry the buffer occupancy, so we send -1 as a feedback
                    if((ttmsg->getBufferOccupancy1()!=-1)||(ttmsg->getBufferOccupancy2()!=-1)||(ttmsg->getBufferOccupancy3()!=-1)){
                        EV << "Error. Opnode id "<<getIndex()<<" ttmsg->getBufferoccupancy1()!=-1 at " << simTime() << "\n";
                    }


                }else if (1 == ttmsg->getType()){
                    //talk message is received. The master is listening. The gateway will send data if there is a packet in the queue

                    scheduleAt(simTime() + offset, send_data_buffer);
                    last_received_talk_control_for_this_gateway=ttmsg->getCounter();
                    EV << "Scheduled Opnode id "<<getIndex()<<" to send data packet in slot number "<<ttmsg->getCounter()<<" at time " << simTime() + offset << " with offset "<<offset<<" if there is a packet in the buffer at " << simTime() << "\n";

                    if((ttmsg->getBufferOccupancy1()!=-1)||(ttmsg->getBufferOccupancy2()!=-1)||(ttmsg->getBufferOccupancy3()!=-1)){
                        EV << "Error. Opnode id "<<getIndex()<<" ttmsg->getBufferoccupancy()!=-1 at " << simTime() << "\n";
                    }

                }else{
                    EV << "Error. Unknown control packet at " << simTime() << "\n";
                }
            }


            send(msg, "control_out");
        }



    }else if (msg->arrivedOn("data_in")){
        //received a data message from optical ring

        inet::EthernetIIFrame *msg2 = check_and_cast<inet::EthernetIIFrame *>(msg);
        CoRE4INET::OpticalDataPacket *msg4 = check_and_cast<CoRE4INET::OpticalDataPacket *>(msg2->getEncapsulatedPacket()->getEncapsulatedPacket());
        EV << "Opnode id "<<getIndex()<<" finished receiving a data message from optical ring in slot "<<msg4->getCounter()<<" at " << simTime() << "\n";


        std::string destination=msg2->getDest().str();
        int core_destination=std::stoi (msg2->getDest().str().substr (9,2));
        int edge_destination=std::stoi (msg2->getDest().str().substr (12,2));


        EV << "its destination is " << destination << " core_destination "<<core_destination<< " edge_destination "<<edge_destination<< " \n";


        if (core_destination == getIndex()) {//data packet is for this gateway or master node
            EV << "Opnode id "<<core_destination<<" finished receiving data packet to send to the edge node at " << simTime() << "\n";
            if((!msg2->getDest().str().compare("0A-10-00-00-00-05"))||(!msg2->getDest().str().compare("0A-10-00-00-00-04"))){
                EV << "Message with ID "<<msg->getId()<<" arrived master gateway for edge "<<msg2->getDest().str()<<" sent from " << getSimulation()->getModule(msg->getSenderModuleId())->getFullPath() << " has latency "<<simTime()-msg->getCreationTime()<< " at " << simTime() << " and was created " << msg->getCreationTime() << "\n";
            }

            if(0 == getIndex()){ //arrived the master node



                //index 0 is master node, so it is ignored. Gateway index starts from 1
                if(((msg4->getGatewayID()>0)&&(msg4->getBufferOccupancy1()>-1)&&(msg4->getBufferOccupancy2()>-1))){
                    EV << "Master Opnode id "<<getIndex()<<" returned control from "<<msg4->getGatewayID()<<" buffer size1 "<<msg4->getBufferOccupancy1()<<" buffer size2 "<<msg4->getBufferOccupancy2()<<" buffer size3 "<<msg4->getBufferOccupancy3()<<" at " << simTime() << "\n";
                    bufferstats1[msg4->getGatewayID()]=msg4->getBufferOccupancy1();
                    bufferstats2[msg4->getGatewayID()]=msg4->getBufferOccupancy2();
                    bufferstats3[msg4->getGatewayID()]=msg4->getBufferOccupancy3();
                }

                if(msg4->getStreamID()==-1){
                    //This is an empty slot for sending the gateway statistics. Drop it.

                    EV << "Empty slot returned the master node with bitrate info "<<msg4->getRate()<<" with StreamID "<<msg4->getStreamID();
                    delete msg;

                }else{
                    //Slot returned to master with a packet and gateway info.


                    EV << "Slot returned to master with a packet and gateway info with bitrate info "<<msg4->getRate()
                                                    <<" with StreamID "<<msg4->getStreamID()
                                                    <<" with size "<<msg2->getByteLength()
                                                    <<" waiting time info "<<msg4->getWaitingTime()
                                                    <<" gateway arrival "<<msg4->getGatewayArrivalTime()
                                                    <<" gateway departure "<<msg4->getGatewayDepatureTime()
                                                    <<" gateway delay "<<msg4->getGatewayDepatureTime()-msg4->getGatewayArrivalTime()
                                                    <<" DepartureTime "<<msg4->getDepartureTime()
                                                    <<" NextDepartureTime "<<msg4->getNextDepartureTime()
                                                    <<" BackboneDelay "<<BackboneDelay
                                                    <<" GatewayID "<<msg4->getGatewayID()
                                                    //<<" Current Slot no "<<(simTime()-BackboneDelay-timerInterval-controlPacketDuration)/timerInterval
                                                    <<" Should schedule to next Slot time "<<(simTime()-BackboneDelay-timerInterval-controlPacketDuration)+(msg4->getNextDepartureTime()-msg4->getDepartureTime())-(msg4->getGatewayDepatureTime()-msg4->getGatewayArrivalTime())
                                                    <<" Should schedule to next Slot no "<<ceil(((simTime()-BackboneDelay-timerInterval-controlPacketDuration)+(msg4->getNextDepartureTime()-msg4->getDepartureTime())-(msg4->getGatewayDepatureTime()-msg4->getGatewayArrivalTime()))/timerInterval)
                                                    <<" at " << simTime() << "\n";

                    //reserve the slot in the master node using next departure time in the packet

                    if(msg4->getGatewayID()>0){
                        if(msg4->getNextDepartureTime()>0){


                            int priority=0;
                            if(msg2->getByteLength()==64){ //currently assume that small packets are control packets with highest priority
                                priority=1;//highest priority
                            }else if(msg4->getNextDepartureTime()>0){ //currently assume that video packets have second highest priority. They are big but they have next departure time information
                                priority=2;
                            }else{
                                priority=3;
                            }

                            int NextSlot=ceil(((simTime()-BackboneDelay-timerInterval-controlPacketDuration)+(msg4->getNextDepartureTime()-msg4->getDepartureTime())-(msg4->getGatewayDepatureTime()-msg4->getGatewayArrivalTime()))/timerInterval);

                            while(SlotAssignmentPriorityList[NextSlot]<=priority){
                                EV << "Search SlotAssignmentPriorityList[NextSlot] "<<SlotAssignmentPriorityList[NextSlot]<<" priority "<<priority<< "\n";
                                NextSlot=NextSlot+1;
                            }

                            EV << "SlotAssignmentPriorityList[NextSlot] "<<SlotAssignmentPriorityList[NextSlot]<< "\n";

                            if(SlotAssignmentPriorityList[NextSlot]!=100){
                                //100 means that the slot is empty.
                                //The selected slot has a reservation for a packet with a lower priority. We should delay the reservation of this packet with lower priority.

                                int priority2=SlotAssignmentPriorityList[NextSlot];
                                int GatewayID2=SlotAssignmentList[NextSlot];
                                int NextSlot2=NextSlot+1;

                                int finished=0;
                                while(finished==0){
                                    while(SlotAssignmentPriorityList[NextSlot2]<priority2){
                                        NextSlot2=NextSlot2+1;
                                    }
                                    if(SlotAssignmentPriorityList[NextSlot2]==100){
                                        finished=1;

                                        SlotAssignmentPriorityList[NextSlot2]=priority2;
                                        SlotAssignmentList[NextSlot2]=GatewayID2;
                                    }else{
                                        //The selected slot has a reservation for a packet with a lower priority. We should delay the reservation of this packet with lower priority.
                                        //temporarily store the current slot information that will be replaced
                                        int priority3=SlotAssignmentPriorityList[NextSlot2];
                                        int GatewayID3=SlotAssignmentList[NextSlot2];

                                        SlotAssignmentPriorityList[NextSlot2]=priority2;
                                        SlotAssignmentList[NextSlot2]=GatewayID2;

                                        priority2=priority3;
                                        GatewayID2=GatewayID3;

                                        NextSlot2=NextSlot2+1;

                                        EV << "temporarily store the current slot information that will be replaced NextSlot2 "<<NextSlot2<<" priority2 "<<priority2<<" GatewayID2 "<<GatewayID2<<  "\n";
                                    }

                                }
                                EV << "Total shift "<<NextSlot2-NextSlot<< "\n";

                            }



                            SlotAssignmentList[NextSlot]=msg4->getGatewayID();
                            SlotAssignmentPriorityList[NextSlot]=priority;

                        }
                    }

                    if(simTime()-msg4->getDepartureTime()>0.0000035){
                        EV << "high delay "<<simTime()-msg4->getDepartureTime()<<" at " << simTime() << "\n";
                    }

                    //Send to edge node

                    inet::PPPFrame *pppFrame = new inet::PPPFrame(msg2->getName());
                    pppFrame->setByteLength(inet::PPP_OVERHEAD_BYTES);
                    pppFrame->encapsulate(msg2);

                    if(SendFinishTime[edge_destination]<=simTime()){


                        EV << "Channel empty. set SendFinishTime[edge_destination] "<<simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";
                        SendFinishTime[edge_destination]=simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
                        send(pppFrame, "gate$o",edge_destination);
                    }else{


                        EV << "Channel busy. set SendFinishTime[edge_destination] "<<SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";

                        sendDelayed(pppFrame,SendFinishTime[edge_destination]-simTime(),"gate$o",edge_destination);
                        SendFinishTime[edge_destination]=SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
                    }




                }
            }else{


                //Send to edge node
                inet::PPPFrame *pppFrame = new inet::PPPFrame(msg2->getName());
                pppFrame->setByteLength(inet::PPP_OVERHEAD_BYTES);
                pppFrame->encapsulate(msg2);

                if(SendFinishTime[edge_destination]<=simTime()){


                    EV << "Channel empty. set SendFinishTime[edge_destination] "<<simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";
                    SendFinishTime[edge_destination]=simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
                    send(pppFrame, "gate$o",edge_destination);
                }else{


                    EV << "Channel busy. set SendFinishTime[edge_destination] "<<SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";

                    sendDelayed(pppFrame,SendFinishTime[edge_destination]-simTime(),"gate$o",edge_destination);
                    SendFinishTime[edge_destination]=SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
                }


            }




        }else{
            EV << "Opnode id "<<getIndex()<<" forward data packet with destination " << destination << " core_destination "<<core_destination<< " edge_destination "<<edge_destination<< " at " << simTime() << "\n";

            if((0 == getIndex())&&(core_destination !=0)){
                EV << "ERROR OpNode::handleMessage did a loop but could not arrive destination at " << simTime() << "\n";
            }else{
                send(msg2,"data_out",0);
            }

        }

    }else if (msg->arrivedOn("gate$i")){
        //received a data message from edge node to send to the optical ring


        EV << "Gateway Opnode id "<<getIndex()<<" finished receiving a data message from edge node to send to the optical ring at " << simTime() << "\n";

        inet::PPPFrame *pppFrame = check_and_cast<inet::PPPFrame *>(msg);
        inet::EthernetIIFrame *msg2 = check_and_cast<inet::EthernetIIFrame *>(pppFrame->decapsulate());

        if((!msg2->getDest().str().compare("0A-10-00-00-00-05"))||(!msg2->getDest().str().compare("0A-10-00-00-00-04"))){
            EV << "Message with ID "<<msg->getId()<<" arrived slave gateway from edge for destination "<<msg2->getDest().str()<<" sent from " << getSimulation()->getModule(msg->getSenderModuleId())->getFullPath() << " has latency "<<simTime()-msg->getCreationTime()<< " at " << simTime() << " and was created " << msg->getCreationTime() << "\n";
        }

        //delete pppFrame;
        delete pppFrame;

        std::string destination=msg2->getDest().str();
        int core_destination=std::stoi (msg2->getDest().str().substr (9,2));
        int edge_destination=std::stoi (msg2->getDest().str().substr (12,2));


        EV << "its destination is " << destination << " core_destination "<<core_destination<< " edge_destination "<<edge_destination<< " \n";


        if((core_destination==0)&&(0 == getIndex())){
            //packet destination is connected to the master code node, so send it directly
            EV << "Data packet destination is connected to the master code node, so send it directly. Master Opnode id "<<core_destination<<" received data packet to send to the edge node at " << simTime() << "\n";




            inet::PPPFrame *pppFrame = new inet::PPPFrame(msg2->getName());
            pppFrame->setByteLength(inet::PPP_OVERHEAD_BYTES);
            pppFrame->encapsulate(msg2);

            if(SendFinishTime[edge_destination]<=simTime()){


                EV << "Direct Channel empty. set SendFinishTime[edge_destination] "<<simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";
                SendFinishTime[edge_destination]=simTime()+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
                send(pppFrame, "gate$o",edge_destination);
            }else{


                EV << "Direct Channel busy. set SendFinishTime[edge_destination] "<<SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth<<" previous was "<<SendFinishTime[edge_destination]<<" pppFrame->getByteLength() "<<pppFrame->getByteLength()<<" with edge_destination "<<edge_destination<< " at " << simTime() << "\n";

                sendDelayed(pppFrame,SendFinishTime[edge_destination]-simTime(),"gate$o",edge_destination);
                SendFinishTime[edge_destination]=SendFinishTime[edge_destination]+pppFrame->getByteLength()*8.0/DataWaveBandwidth;
            }


        }else{
            //packet destination is another core node, so buffer and send it via optical ring
            //EV << "Packet size is " << msg2->getByteLength() << "\n";

            CoRE4INET::OpticalDataPacket *msg3 = check_and_cast<CoRE4INET::OpticalDataPacket *>(msg2->getEncapsulatedPacket()->getEncapsulatedPacket());

            msg3->setWaitingTime(simTime());
            msg3->setGatewayArrivalTime(simTime());
            msg3->setGatewayID(getIndex());


            EV << "Save arrival time to the data packet, which arrived gateway "<<getIndex()<<" at " << simTime() << "\n";

            if(msg2->getByteLength()==64){
                //high priority traffic (control)


                int64_t arrival_slot=ceil((simTime()+msg3->getNextDepartureTime()-msg3->getDepartureTime()-first_control_time-offset)/(timerInterval));

                EV << "Should reserve slot no "<<arrival_slot<<" for the next packet that will arrive at "<<simTime()+msg3->getNextDepartureTime()-msg3->getDepartureTime()
                                                        <<" after substracting first_control_time and offset " << simTime()+msg3->getNextDepartureTime()-msg3->getDepartureTime()-first_control_time-offset
                                                        <<" timerInterval " << timerInterval
                                                        <<" division without floor " << (simTime()+msg3->getNextDepartureTime()-msg3->getDepartureTime()-first_control_time-offset)/(timerInterval)
                                                        <<" division with floor " << ceil((simTime()+msg3->getNextDepartureTime()-msg3->getDepartureTime()-first_control_time-offset)/(timerInterval))
                                                        <<" at " << simTime() << "\n";


                ToOpticalRingQueue->insert(msg2);
            }else if(0.0!=msg3->getNextDepartureTime().dbl()){
                //middle priority traffic (video etc.)
                ToOpticalRingQueue2->insert(msg2);
                EV << "Queue middle priority traffic " << simTime() << "\n";
            }else{
                //Best effort traffic (Internet)
                ToOpticalRingQueue3->insert(msg2);
                EV << "Queue Best effort traffic " << simTime() << "\n";
            }

        }

    }else{
        EV << "Error. Unknown packet at " << simTime() << "\n";
    }


}

OpticalControlPacket *OpNode::generateMessage(int64_t counter)
{
    // Produce source and destination addresses.
    int src = getIndex();
    //    int n = getVectorSize();
    //    int dest = intuniform(0, n-2);
    //    if (dest >= src)
    //        dest++;

    char msgname[20];
    sprintf(msgname, "sch %d slot %d", schedulelist[currentslot],currentslot);

    // Create message object and set source and destination field.
    OpticalControlPacket *msg = new OpticalControlPacket(msgname);
    msg->setSource(src);
    msg->setDestination(-1);
    msg->setType(-1);
    msg->setCounter(counter);
    return msg;
}

void OpNode::forwardMessage(OpticalControlPacket *msg)
{
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);

    // Same routing as before: random gate.
    int n = gateSize("gate");
    int k = intuniform(0, n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}


OpNode::~OpNode()
{
    // remove messages
    cMessage *msg;
    while(!ToOpticalRingQueue->isEmpty()) {
        msg = (cMessage *)ToOpticalRingQueue->pop();
        cancelAndDelete(msg);
    }
    delete ToOpticalRingQueue;

    while(!ToOpticalRingQueue2->isEmpty()) {
        msg = (cMessage *)ToOpticalRingQueue2->pop();
        cancelAndDelete(msg);
    }
    delete ToOpticalRingQueue2;

    while(!ToOpticalRingQueue3->isEmpty()) {
        msg = (cMessage *)ToOpticalRingQueue3->pop();
        cancelAndDelete(msg);
    }
    delete ToOpticalRingQueue3;


    delete [] SendFinishTime;

    cancelAndDelete(send_data_buffer);
    cancelAndDelete(control_packet_generator_timer);


}
