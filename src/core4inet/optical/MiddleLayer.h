
#ifndef CORE4INET_MiddleLayer_H_
#define CORE4INET_MiddleLayer_H_

//CoRE4INET
#include "core4inet/applications/AS6802/CTApplicationBase.h"
#include "core4inet/utilities/classes/Scheduled.h"

namespace CoRE4INET {

/**
 * @brief Tic Application used for the rt_tictoc example.
 *
 *
 * @sa ApplicationBase
 * @ingroup Applications AS6802
 *
 * @author Till Steinbach
 */
class MiddleLayer : public virtual CTApplicationBase
{
    protected:
        /**
         * @brief Initialization of the module. Sends activator message
         */
        virtual void initialize() override;
        /**
         * @brief Handles message generation and reception
         */
        virtual void handleMessage(cMessage *msg) override;

    protected:

};

}

#endif
