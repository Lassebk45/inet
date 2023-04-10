//
// Created by lasse on 3/24/23.
//

#include "inet/p10/TwoPhaseCommit.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

Define_Module(TwoPhaseCommit);

void TwoPhaseCommit::initialize(){
    updatePath = par("updatePath");
    nextUpdateTime = par("updateInterval");
    updateTrigger = new cMessage();

    if (nextUpdateTime > SIMTIME_ZERO)
    {
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void TwoPhaseCommit::handleMessage(cMessage* msg){
    if (msg == updateTrigger)
    {
        // If the update file exists load the updates
        if (FILE *file = fopen(updatePath, "r")) {
            fclose(file);

            const cXMLElement * updates = getEnvir()->getXMLDocument(updatePath);
            update(updates);

            // Delete the update file so it is not implemented multiple times.
            // remove(updatePath);
        }

        nextUpdateTime += par("updateInterval");
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void TwoPhaseCommit::update(const cXMLElement * updates)
{
    using namespace xmlutils;
    
    ASSERT(updates);
    ASSERT(!strcmp(updates->getTagName(), "twoPhaseCommit"));
    checkTags(updates, "add remove reclassify");
    for(cXMLElement* entry : updates->getChildrenByTagName("add"))
    {
        LibTable* libTable = (LibTable *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".libTable");
        libTable->updateLibTable(entry);
    }
    for(cXMLElement* entry : updates->getChildrenByTagName("remove"))
    {
        LibTable* libTable = (LibTable *)getModuleByPath(entry->getAttribute("router"))->getModuleByPath(".libTable");
        libTable->updateLibTable(entry);
    }
    
}
}