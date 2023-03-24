//
// Created by lasse on 3/24/23.
//

#include "inet/p10/MeasureWriter.h"
#include "inet/p10/json.hpp"
#include <iomanip>
#include <string>
#include <fstream>

namespace omnetpp {
namespace inet {

Define_Module(MeasureWriter);

void MeasureWriter::receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details)
{
    if (dynamic_cast<cDatarateChannel *>(source)) {
        cDatarateChannel * _source = (cDatarateChannel *) source;
        std::string srcRouter = _source->getSourceGate()->getOwner()->getFullName();
        std::string tgtRouter = _source->getSourceGate()->getNextGate()->getOwner()->getFullName();
        updateUtilization(srcRouter, tgtRouter, d);
    }
}
void MeasureWriter::initialize()
{
    getSimulation()->getSystemModule()->subscribe("utilization", this);
    writeInterval = par("writeInterval");
    
    nextWriteTime = SIMTIME_ZERO + writeInterval;
    scheduleAt(nextWriteTime, writeTrigger);
}

void MeasureWriter::updateUtilization(std::string src, std::string tgt, double utilization)
{
    test[src][tgt] = utilization;
    linkUtilizations[std::make_pair(src, tgt)] = utilization;
}

void MeasureWriter::handleMessage(cMessage* msg)
{
    if (msg == writeTrigger)
    {
        double utilizationSnapshotTime = SIMTIME_DBL(nextWriteTime);
        test["timestamp"] = utilizationSnapshotTime;
        for (auto it = linkUtilizations.begin(); it != linkUtilizations.end(); ++it)
        {
            //printf("%s -> %s: %f\n", it->first.first.c_str(), it->first.second.c_str(), it->second);
            std::ofstream o("utilization.json");
            o << std::setw(4) << test << std::endl;
        }
        nextWriteTime = nextWriteTime + writeInterval;
        scheduleAt(nextWriteTime, writeTrigger);
    }
}
}
}