//
// Created by lasse on 5/30/23.
//

#ifndef INET_DYNAMICWEIGHTS_H
#define INET_DYNAMICWEIGHTS_H

#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

class INET_API DynamicWeights : public cSimpleModule {
    protected:
        cMessage updateTrigger;
        const char* updatePath;
        const char* initialPath;
        simtime_t nextUpdateTime;
        bool initialWeights = true;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;
        virtual void applyNewWeights(const cXMLElement *dynamicWeights);
};

}

#endif //INET_DYNAMICWEIGHTS_H
