//
// Created by lasse on 3/24/23.
//

#ifndef INET_MEASUREWRITER_H
#define INET_MEASUREWRITER_H

#include "inet/p10/json.hpp"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

class INET_API MeasureWriter : public cSimpleModule, public cListener {
    protected:
        simsignal_t utilSignal;
        simsignal_t sendIntervalChangedSignal;
        simsignal_t libTableChangedSignal;

        nlohmann::json linkUtilizations;
        nlohmann::json demands;
        nlohmann::json libTables;

        simtime_t lastUpdate = SIMTIME_ZERO;
        simtime_t writeInterval;
        cMessage* writeTrigger = new cMessage();
        simtime_t nextWriteTime;
    protected:
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
        virtual void initialize() override;
        virtual void updateUtilization(std::string src, std::string tgt, double utilization);
        virtual void updateDemands(UdpBasicApp* app, double sendInterval);
        virtual void updateLibTable(LibTable * libTable);
        virtual void handleMessage(cMessage* msg) override;
        virtual void writeUtilization();
        virtual void writeDemands();
        virtual void writeLibTables();
};

std::string labelOpCodeToString(LabelOpCode code);
}
#endif //INET_MEASUREWRITER_H