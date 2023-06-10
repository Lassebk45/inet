//
// Created by lasse on 3/24/23.
//

#ifndef INET_TWOPHASECOMMIT_H
#define INET_TWOPHASECOMMIT_H

#include "inet/p10/json.hpp"
#include "inet/p10/TwoPhaseCommitMsg_m.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/common/XMLUtils.h"

namespace inet {

class INET_API TwoPhaseCommit

: public cSimpleModule {
protected:
cMessage *updateTrigger;
cMessage *firstPhaseMsg;
cMessage *secondPhaseMsg;
TwoPhaseCommitMsg *initateUpdate;
const char *updatePath;
const char *updateTimePath;
simtime_t nextUpdateTime;
double updateInterval;
const cXMLElement *updates;


protected:

virtual void initialize()

override;

virtual void handleMessage(cMessage *msg)

override;

virtual void firstPhase();

virtual void secondPhase();

};

}
#endif //INET_TWOPHASECOMMIT_H
