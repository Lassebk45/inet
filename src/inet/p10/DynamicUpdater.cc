//
// Created by lasse on 3/24/23.
//

#include "inet/p10/DynamicUpdater.h"

namespace inet {

Define_Module(DynamicUpdater);

void DynamicUpdater::initialize(){
    updatePath = par("updatePath");
    nextUpdateTime = par("updateInterval");
    updateTrigger = new cMessage();

    if (nextUpdateTime > SIMTIME_ZERO)
    {
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void DynamicUpdater::handleMessage(cMessage* msg){
    if (msg == updateTrigger)
    {
        // If the update file exists load the updates
        if (FILE *file = fopen(updatePath, "r")) {
            fclose(file);

            const cXMLElement * updates = getEnvir()->getXMLDocument(updatePath);
            update(updates);

            printf("%s\n", updates->str().c_str());

            // Delete the update file so it is not implemented multiple times.
            remove(updatePath);
        }

        nextUpdateTime += par("updateInterval");
        scheduleAt(nextUpdateTime, updateTrigger);
    }
}

void DynamicUpdater::update(const cXMLElement * updates)
{
    
}
}