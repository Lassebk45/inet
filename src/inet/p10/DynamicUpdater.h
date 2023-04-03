//
// Created by lasse on 3/24/23.
//

#ifndef INET_DYNAMICUPDATER_H
#define INET_DYNAMICUPDATER_H

#include "inet/p10/json.hpp"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

class INET_API DynamicUpdater : public cSimpleModule {
    protected:
        cMessage* updateTrigger;
        const char* updatePath;
        simtime_t nextUpdateTime;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;
        virtual void update(const cXMLElement * updates);
};

}
#endif //INET_DYNAMICUPDATER_H
