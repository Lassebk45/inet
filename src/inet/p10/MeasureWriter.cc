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
    else if (signalID == sendIntervalChangedSignal)
    {
        UdpBasicApp* udpApp = (UdpBasicApp*) details;
        updateDemands(udpApp, d);
    }
}

void MeasureWriter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (signalID == libTableChangedSignal)
    {
        LibTable* libTableModule = (LibTable *) obj;
        updateLibTable(libTableModule);
    }
}
void MeasureWriter::initialize()
{
    utilSignal = registerSignal("utilization");
    sendIntervalChangedSignal = registerSignal("sendIntervalChanged");
    libTableChangedSignal = registerSignal("libTableChanged");

    getSimulation()->getSystemModule()->subscribe(utilSignal, this);
    getSimulation()->getSystemModule()->subscribe(sendIntervalChangedSignal, this);
    getSimulation()->getSystemModule()->subscribe(libTableChangedSignal, this);
    writeInterval = par("writeInterval");
    
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

void MeasureWriter::updateLibTable(LibTable * libTable)
{
    /*std::string router = libTable->getParentModule()->getFullName();
    for (auto libEntry : libTable->getLibTable())
    {
        for (LibTable::ForwardingEntry forwardingEntry : libEntry.entries)
        {
            // The following lines is an extremely ad hoc way of finding the target router of the forwarding entry.
            std::string nextHop;
            std::string outInterface = forwardingEntry.outInterface;
            if (outInterface == "mlo0")
            {
                nextHop = router;
            }
            else if (outInterface.substr(0,3) == "ppp")
            {
                int pppGate = std::atoi(outInterface.substr(3).c_str());
                nextHop = libTable->getParentModule()->gate("pppg$o", pppGate)->getNextGate()->getOwnerModule()->getFullName();
            }
            else
            {
                throw "exception";
            }
            std::vector<std::pair<int, std::string>> labelOperations;
            labelOperations.resize(forwardingEntry.outLabel.size());
            std::transform(forwardingEntry.outLabel.begin(), forwardingEntry.outLabel.end(), labelOperations.begin(),
                           [](LabelOp l) -> std::pair<int, std::string> {return std::make_pair(l.label, labelOpCodeToString(l.optcode));});

            libTables[router][std::to_string(libEntry.inLabel)][std::to_string(forwardingEntry.priority)] = {nextHop, labelOperations};
        }
    }*/
}

void MeasureWriter::handleMessage(cMessage* msg)
{
    if (msg == writeTrigger)
    {
        writeUtilization();
        writeDemands();
        writeLibTables();
        nextWriteTime = nextWriteTime + writeInterval;
        scheduleAt(nextWriteTime, writeTrigger);
    }
}

void MeasureWriter::writeUtilization()
{
    linkUtilizations["timestamp"] = SIMTIME_DBL(nextWriteTime);
    std::ofstream o("utilization.json");
    o << std::setw(4) << linkUtilizations << std::endl;
}

void MeasureWriter::writeDemands()
{
    demands["timestamp"] = SIMTIME_DBL(nextWriteTime);
    std::ofstream o("demands.json");
    o << std::setw(4) << demands << std::endl;
}

void MeasureWriter::writeLibTables()
{
    demands["timestamp"] = SIMTIME_DBL(nextWriteTime);
    std::ofstream o("libTables.json");
    o << std::setw(4) << libTables << std::endl;
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