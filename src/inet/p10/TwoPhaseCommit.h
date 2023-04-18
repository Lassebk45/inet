//
// Created by lasse on 3/24/23.
//

#ifndef INET_TWOPHASECOMMIT_H
#define INET_TWOPHASECOMMIT_H

#include "inet/p10/json.hpp"
#include "inet/p10/TwoPhaseCommitMsg_m.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

class INET_API TwoPhaseCommit : public cSimpleModule {
    protected:
        cMessage* updateTrigger;
        TwoPhaseCommitMsg* secondPhaseMsg;
        const char* updatePath;
        simtime_t nextUpdateTime;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;
        virtual void firstPhase(const cXMLElement * updates);
        virtual void secondPhase(const cXMLElement * updates);
};

}
#endif //INET_TWOPHASECOMMIT_H
