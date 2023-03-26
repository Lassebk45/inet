//
// Created by lasse on 3/24/23.
//

#include "inet/p10/MeasureWriter.h"
#include "inet/p10/json.hpp"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include <iomanip>
#include <string>
#include <fstream>

namespace inet {

Define_Module(MeasureWriter);

void MeasureWriter::receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details)
{
    if (signalID == utilSignal)
    {
        cDatarateChannel* _source = (cDatarateChannel *) source;
        std::string srcRouter = _source->getSourceGate()->getOwner()->getFullName();
        std::string tgtRouter = _source->getSourceGate()->getNextGate()->getOwner()->getFullName();
        updateUtilization(srcRouter, tgtRouter, d);
    }
}

void MeasureWriter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (signalID == sendIntervalChangedSignal)
    {
        UdpBasicApp* _obj = (UdpBasicApp*) obj;
    }
}
void MeasureWriter::initialize()
{
    utilSignal = registerSignal("utilization");
    sendIntervalChangedSignal = registerSignal("sendIntervalChanged");
    getSimulation()->getSystemModule()->subscribe(utilSignal, this);
    getSimulation()->getSystemModule()->subscribe(sendIntervalChangedSignal, this);
    getSimulation()->getSystemModule()->subscribe(sendIntervalChangedSignal, this);
    writeInterval = par("writeInterval");
    
    nextWriteTime = SIMTIME_ZERO + writeInterval;
    scheduleAt(nextWriteTime, writeTrigger);
}

void MeasureWriter::updateUtilization(std::string src, std::string tgt, double utilization)
{
    linkUtilizations[src][tgt] = utilization;
}

void MeasureWriter::handleMessage(cMessage* msg)
{
    if (msg == writeTrigger)
    {
        writeUtilization();
        writeDemands();
        nextWriteTime = nextWriteTime + writeInterval;
        scheduleAt(nextWriteTime, writeTrigger);
    }
}

void MeasureWriter::writeUtilization()
{
    linkUtilizations["timestamp"] = SIMTIME_DBL(nextWriteTime);
    for (auto it = linkUtilizations.begin(); it != linkUtilizations.end(); ++it)
    {
        std::ofstream o("utilization.json");
        o << std::setw(4) << linkUtilizations << std::endl;
    }
}

void MeasureWriter::writeDemands()
{

}
}