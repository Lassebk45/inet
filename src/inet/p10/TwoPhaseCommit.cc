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

namespace inet {

Define_Module(TwoPhaseCommit);


void TwoPhaseCommit::initialize(){
    updatePath = par("updatePath");
    nextUpdateTime = par("updateInterval");
    updateTrigger = new cMessage();
    secondPhaseMsg = new TwoPhaseCommitMsg();

    if (nextUpdateTime > SIMTIME_ZERO)
    {
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void TwoPhaseCommit::handleMessage(cMessage* msg){
    if (msg == updateTrigger)
    {
        getEnvir()->forgetXMLDocument(updatePath);
        
        FILE *file;
        // When the update file exists load the updates
        while (!(file = fopen(updatePath, "r")))
        {
            sleep(1);
            std::cout << "Waiting for updateFile" << std::endl;
        }
        fclose(file);
        const cXMLElement * updates = getEnvir()->getXMLDocument(updatePath);
        firstPhase(updates);
        cancelEvent(secondPhaseMsg);
        //Ipv4NetworkConfigurator* ipv4NetworkConfigurator = (Ipv4NetworkConfigurator *) getModuleByPath("configurator");
        //simtime_t networkFlushTime = ipv4NetworkConfigurator->networkFlushTime(par("highestLatencyMS"));
        //secondPhaseMsg->setUpdateElement(updates);
        //scheduleAfter(networkFlushTime, secondPhaseMsg);
        secondPhaseMsg->setUpdateElement(updates);
        simtime_t updateInterval = par("updateInterval");
        scheduleAfter(updateInterval * 0.9, secondPhaseMsg);
        // Delete the update file so it is not implemented multiple times.
        remove(updatePath);
    
        nextUpdateTime += par("updateInterval");
        scheduleAt(nextUpdateTime, updateTrigger);
    }
    else if (msg == secondPhaseMsg)
    {
        //TwoPhaseCommitMsg* updateMessage = (TwoPhaseCommitMsg*) msg;
        secondPhase(secondPhaseMsg->getUpdateElement());
    }
}

void TwoPhaseCommit::firstPhase(const cXMLElement * updates)
{
    
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

void TwoPhaseCommit::secondPhase(const cXMLElement * updates)
{
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