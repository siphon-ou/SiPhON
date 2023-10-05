
#ifndef CORE4INET_OpBGTrafficSinkApp_H_
#define CORE4INET_OpBGTrafficSinkApp_H_

//CoRE4INET
#include "core4inet/applications/trafficsink/base/TrafficSinkApp.h"
//INET
#include "inet/linklayer/common/MACAddress.h"

namespace CoRE4INET {

/**
 * @brief Traffic sink application used for statistics collection of best-effort traffic.
 *
 *
 * @sa TrafficSinkApp
 * @ingroup Applications
 *
 * @author Till Steinbach, Sandra Reider
 */
class OpBGTrafficSinkApp : public virtual TrafficSinkApp
{
    private:
        unsigned int received;
    public:
        OpBGTrafficSinkApp();
    protected:
        inet::MACAddress address;
        /**
         * @brief Initialization of the module.
         */
        virtual void initialize() override;

        /**
         * @brief collects incoming message and writes statistics.
         *
         * @param msg incoming frame
         */
        virtual void handleMessage(cMessage *msg) override;

        /**
         * @brief Indicates a parameter has changed.
         *
         * @param parname Name of the changed parameter or nullptr if multiple parameter changed.
         **/
        virtual void handleParameterChange(const char* parname) override;
};

} //namespace

#endif
