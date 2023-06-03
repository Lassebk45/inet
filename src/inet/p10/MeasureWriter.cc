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
#include <sstream>

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
    if (signalID == linkFailure){
        cGate* gate = (cGate *)obj;
        cModule *src = (cModule *)gate->getOwner();
        cModule *dest = (cModule *)gate->getNextGate()->getOwner();
        std::string srcName = src->getName();
        std::string destName = dest->getName();
        downLinks.insert(std::make_pair(srcName, destName));
    }
    if (signalID == linkReconnected){
        cGate* gate = (cGate *)obj;
        cModule *src = (cModule *)gate->getOwner();
        cModule *dest = (cModule *)gate->getNextGate()->getOwner();
        std::string srcName = src->getName();
        std::string destName = dest->getName();
        downLinks.erase(std::make_pair(srcName, destName));
    }
}
void MeasureWriter::initialize()
{
    utilSignal = registerSignal("utilization");
    sendIntervalChangedSignal = registerSignal("sendIntervalChanged");
    libTableChangedSignal = registerSignal("libTableChanged");
    linkFailure = registerSignal("linkFailure");
    linkReconnected = registerSignal("linkReconnected");

    getSimulation()->getSystemModule()->subscribe(utilSignal, this);
    getSimulation()->getSystemModule()->subscribe(sendIntervalChangedSignal, this);
    //getSimulation()->getSystemModule()->subscribe(libTableChangedSignal, this);
    getSimulation()->getSystemModule()->subscribe(POST_MODEL_CHANGE, this);
    getSimulation()->getSystemModule()->subscribe(linkFailure, this);
    getSimulation()->getSystemModule()->subscribe(linkReconnected, this);
    writeInterval = par("writeInterval");
    demandPath = par("demandPath");
    utilizationPath = par("utilizationPath");
    linkFailuresPath = par("linkFailuresPath");
    
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
    std::cout << "Wrote link utilization" << std::endl;
    std::ofstream oo(demandPath);
    oo << std::setw(4) << demands << std::endl;
    oo.close();
    std::cout << "Wrote demands" << std::endl;
    
    std::ostringstream outString;
    outString << "[";
    for (std::pair<std::string, std::string> link: downLinks){
        outString << "[\"" << link.first << "\", \"" << link.second << "\"], ";
    }
    outString << "]";
    
    std::ofstream ooo(linkFailuresPath);
    ooo << outString.str();
    ooo.close();
    std::cout << "Wrote down links" << std::endl;
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