//
// Created by lasse on 3/24/23.
//

#ifndef INET_MEASUREWRITER_H
#define INET_MEASUREWRITER_H

#include "inet/p10/json.hpp"

namespace omnetpp {
namespace inet {

class LinkUtilization {
    public:
        std::string src = nullptr;
        std::string tgt = nullptr;
        double utilization = 0;
        LinkUtilization(std::string src, std::string tgt, double utilization){this->src = src; this->tgt = tgt; this->utilization = utilization;}
};

class MeasureWriter : public cSimpleModule, public cListener {
    private:
        std::map<std::pair<std::string, std::string>, double> linkUtilizations;
        nlohmann::json test;
        simtime_t lastUpdate = SIMTIME_ZERO;
        simtime_t writeInterval;
        cMessage* writeTrigger = new cMessage();
        simtime_t nextWriteTime;
    protected:
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
        virtual void initialize() override;
        virtual void updateUtilization(std::string src, std::string tgt, double utilization);
        virtual void handleMessage(cMessage* msg) override;
};

}
}
#endif //INET_MEASUREWRITER_H
