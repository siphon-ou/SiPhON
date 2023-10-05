
#ifndef CORE4INET_OpBGTrafficSourceApp_H_
#define CORE4INET_OpBGTrafficSourceApp_H_

//CoRE4INET
#include "core4inet/applications/trafficsource/base/TrafficSourceAppBase.h"

//INET
#include "inet/linklayer/common/MACAddress.h"

namespace CoRE4INET {

/**
 * @brief Traffic generator application for best-effor traffic.
 *
 * @sa TrafficSourceAppBase
 * @ingroup Applications
 *
 * @author Till Steinbach
 */
class OpBGTrafficSourceApp : public virtual TrafficSourceAppBase
{
    private:
        /**
         * Checks whether the parameters were already initialized
         */
        bool parametersInitialized;

        /**
         * @brief caches sendInterval parameter
         */
        simtime_t sendInterval;

        /**
         * @brief caches destAddress parameter
         */
        inet::MACAddress destAddress;

        int streamID;

        bool NextPacketDepatureIsKnown;

    public:
        /**
         * @brief Constructor of OpBGTrafficSourceApp
         */
        OpBGTrafficSourceApp();

    protected:
        /**
         * @brief Signal that is emitted each time the send interval is used.
         */
        static simsignal_t sigSendInterval;

        /**
         * @brief Initialization of the module.
         */
        virtual void initialize() override;

        /**
         * @brief handle self messages to send frames
         *
         *
         * @param msg incoming self messages
         */
        virtual void handleMessage(cMessage *msg) override;

        /**
         * @brief Generates and sends a new Message.
         *
         * The message is sent to the buffer with the ct_id defined in parameter ct_id of the module.
         * The message kind is defined by the buffer-type (RC/TT) of the buffer the message is sent to.
         * The size is defined by the payload parameter of the module.
         */
        virtual void sendMessage();

        /**
         * @brief Indicates a parameter has changed.
         *
         * @param parname Name of the changed parameter or nullptr if multiple parameter changed.
         *         */
        virtual void handleParameterChange(const char* parname) override;

        inet::MACAddress getDestAddress();
};

} //namespace

#endif
