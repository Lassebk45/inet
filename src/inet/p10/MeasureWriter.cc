//
// Created by lasse on 3/24/23.
//

#include "inet/p10/MeasureWriter.h"
#include "inet/p10/json.hpp"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/networklayer/mpls/LibTable.h"

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
    if (signalID == sendIntervalChangedSignal)
    {
        UdpBasicApp* udpApp = (UdpBasicApp*) details;
        updateDemands(udpApp, d);
    }
}

void MeasureWriter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    ;
}
void MeasureWriter::initialize()
{
    utilSignal = registerSignal("utilization");
    sendIntervalChangedSignal = registerSignal("sendIntervalChanged");
    libTableChangedSignal = registerSignal("libTableChanged");

    getSimulation()->getSystemModule()->subscribe(utilSignal, this);
    getSimulation()->getSystemModule()->subscribe(sendIntervalChangedSignal, this);
    getSimulation()->getSystemModule()->subscribe(libTableChangedSignal, this);
    getSimulation()->getSystemModule()->subscribe(POST_MODEL_CHANGE, this);
    writeInterval = par("writeInterval");
    demandPath = par("demandPath");
    utilizationPath = par("utilizationPath");
    
    nextWriteTime = SIMTIME_ZERO + writeInterval;
    scheduleAt(nextWriteTime, writeTrigger);
}

void MeasureWriter::updateUtilization(std::string src, std::string tgt, double utilization)
{
    linkUtilizations[src][tgt] = utilization;
}

void MeasureWriter::updateDemands(UdpBasicApp* app, double sendInterval)
{
    // Get the source router
    const char * source = app->getParentModule()->gate("pppg$o", 0)->getNextGate()->getOwnerModule()->getFullName();
    // Get list of target Target routers
    std::vector<std::string> targetRouters;
    for (std::string tgtAdress : app->getDestAddressStr())
    {
        std::string tgtRouter = getModuleByPath(tgtAdress.c_str())->gate("pppg$o", 0)->getNextGate()->getOwnerModule()->getFullName();
        targetRouters.push_back(tgtRouter);
    }

    demands[source][targetRouters[0]] = sendInterval;
}

void MeasureWriter::handleMessage(cMessage* msg)
{
    if (msg == writeTrigger)
    {
        writeMeasures();
        nextWriteTime = nextWriteTime + writeInterval;
        scheduleAt(nextWriteTime, writeTrigger);
    }
}

void MeasureWriter::writeMeasures()
{
    std::ofstream o(utilizationPath);
    o << std::setw(4) << linkUtilizations << std::endl;
    o.close();
    std::ofstream oo(demandPath);
    oo << std::setw(4) << demands << std::endl;
    oo.close();
}

std::string labelOpCodeToString(LabelOpCode code)
{
    if (code == PUSH_OPER)
        return "push";
    else if (code == SWAP_OPER)
        return "swap";
    else if (code == POP_OPER)
        return "pop";
    else
        throw "exception in labelOpCodeToString()";
}

}