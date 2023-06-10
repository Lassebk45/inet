//
// Created by lasse on 3/24/23.
//

#include "inet/p10/TwoPhaseCommit.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/rsvpte/RsvpClassifier.h"
#include "inet/networklayer/rsvpte/RsvpTe.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"
#include "inet/p10/TwoPhaseCommitMsg_m.h"
#include "inet/linklayer/ppp/Ppp.h"
#include <ctime>
#include <iostream>
#include <fstream>

namespace inet {

Define_Module(TwoPhaseCommit);


void TwoPhaseCommit::initialize(){
    updatePath = par("updatePath");
    updateTimePath = par("updateTimePath");
    nextUpdateTime = par("updateInterval");
    updateInterval = par("updateInterval");
    updateTrigger = new cMessage();
    firstPhaseMsg = new cMessage();
    secondPhaseMsg = new cMessage();
    //secondPhaseMsg = new TwoPhaseCommitMsg();
    
    if (nextUpdateTime > SIMTIME_ZERO)
    {
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void TwoPhaseCommit::handleMessage(cMessage* msg){
    if (msg == updateTrigger)
    {
        // When the update file exists load the updates
        std::cout << "Waiting for 2-phase-commit file at time: " << simTime() << std::endl;
        while (!(( access( updatePath, F_OK ) != -1 )) or !(( access( updateTimePath, F_OK ) != -1 )))
        {
            sleep(1);
        }
        std::cout << "2-phase-commit file received" << std::endl;
        double iterationTime;
        {
            std::ifstream fin(updateTimePath);
            fin >> iterationTime;
        }
        getEnvir()->forgetXMLDocument(updatePath);
        updates = getEnvir()->getXMLDocument(updatePath);
        
        simtime_t timeUntilFirstPhase = iterationTime < updateInterval ? SIMTIME_ZERO : iterationTime - updateInterval;
        simtime_t timeUntilSecondPhase = timeUntilFirstPhase + updateInterval * 0.5;
        simtime_t timeUntilNewUpdate = timeUntilFirstPhase + updateInterval;
        
        std::cout << "Routing generation time: " << iterationTime << std::endl;
        std::cout << "Scheduling update in:" << timeUntilFirstPhase << " seconds" << std::endl;
        
        scheduleAfter(timeUntilFirstPhase, firstPhaseMsg);
        scheduleAfter(timeUntilSecondPhase, secondPhaseMsg);
        scheduleAfter(timeUntilNewUpdate, updateTrigger);
        //Ipv4NetworkConfigurator* ipv4NetworkConfigurator = (Ipv4NetworkConfigurator *) getModuleByPath("configurator");
        //simtime_t networkFlushTime = ipv4NetworkConfigurator->networkFlushTime(par("highestLatencyMS"));
        //secondPhaseMsg->setUpdateElement(updates);
        //scheduleAfter(networkFlushTime, secondPhaseMsg);
        
    }
    else if (msg == firstPhaseMsg){
        firstPhase();
        remove(updatePath);
    }
    else if (msg == secondPhaseMsg)
        secondPhase();
}

void TwoPhaseCommit::firstPhase()
{
    std::cout << "Implementing first phase at time:" << simTime() << std::endl;
    using namespace xmlutils;
    ASSERT(updates);
    ASSERT(!strcmp(updates->getTagName(), "twoPhaseCommit"));
    // Assert that all operations are add rule, remove rule or reclassify entry label of flow
    checkTags(updates, "add remove swap reclassify");
    // Apply add rules
    for(cXMLElement* entry : updates->getChildrenByTagName("add"))
    {
        LibTable* libTable = (LibTable *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".libTable");
        libTable->updateLibTable(entry);
    }
    // Apply swap rules
    for(cXMLElement* entry : updates->getChildrenByTagName("swap"))
    {
        LibTable* libTable = (LibTable *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".libTable");
        libTable->updateLibTable(entry);
    }
    // Apply reclassify rules
    for(cXMLElement* entry : updates->getChildrenByTagName("reclassify"))
    {
        checkTags(entry, "label source destination");
        RsvpClassifier* rsvpClassifier = (RsvpClassifier *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".classifier");
        rsvpClassifier->updateFecEntry(entry);
    }
}

void TwoPhaseCommit::secondPhase()
{
    std::cout << "Implementing second phase at time:" << simTime() << std::endl;
    using namespace xmlutils;
    ASSERT(updates);
    ASSERT(!strcmp(updates->getTagName(), "twoPhaseCommit"));
    // Assert that all operations are add rule, remove rule or reclassify entry label of flow
    //checkTags(updates, "add remove swap reclassify");
    // Apply remove rules
    for(cXMLElement* entry : updates->getChildrenByTagName("remove"))
    {
        LibTable* libTable = (LibTable *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".libTable");
        libTable->updateLibTable(entry);
    }
}
}