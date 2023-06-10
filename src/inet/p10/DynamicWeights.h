//
// Created by lasse on 5/30/23.
//

#ifndef INET_DYNAMICWEIGHTS_H
#define INET_DYNAMICWEIGHTS_H

#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

class INET_API DynamicWeights : public cSimpleModule {
    protected:
        cMessage initialUpdateTrigger;
        cMessage updateTrigger;
        cMessage applyWeightsMsg;
        const char* updatePath;
        simtime_t nextUpdateTime;
        cXMLElement * dynamicWeights;
        const char *updateTimePath;
        double updateInterval;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;
        virtual void applyNewWeights();
};

}

#endif //INET_DYNAMICWEIGHTS_H
