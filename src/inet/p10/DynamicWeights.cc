#include "inet/p10/DynamicWeights.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/rsvpte/RsvpClassifier.h"
#include "inet/networklayer/rsvpte/RsvpTe.h"
#include "inet/common/XMLUtils.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>

namespace inet {

Define_Module(DynamicWeights);

void DynamicWeights::initialize() {
    // Load initial weights
    updateTimePath = par("updateTimePath");
    updateInterval = par("updateInterval");
    updatePath = par("updatePath");
    
    
    nextUpdateTime = SIMTIME_ZERO;
    scheduleAt(nextUpdateTime, &initialUpdateTrigger);
    nextUpdateTime = updateInterval;
    scheduleAt(nextUpdateTime, &updateTrigger);
}

void DynamicWeights::handleMessage(cMessage *msg) {
    // When the update file exists load the updates
    if (msg == &initialUpdateTrigger){
        const char* initialPath = par("initialPath");
        getEnvir()->forgetXMLDocument(initialPath);
        dynamicWeights = getEnvir()->getXMLDocument(initialPath);
        applyNewWeights();
    }
    else if (msg == &updateTrigger) {
        std::cout << "Waiting for dynamic weights file..." << std::endl;
        while (!((access(updatePath, F_OK) != -1)) or !((access(updateTimePath, F_OK) != -1))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << "Dynamic weights file received." << std::endl;
        
        double iterationTime;
        {
            std::ifstream fin(updateTimePath);
            fin >> iterationTime;
        }
        getEnvir()->forgetXMLDocument(updatePath);
        dynamicWeights = getEnvir()->getXMLDocument(updatePath);
        simtime_t applyWeightsIn = iterationTime < updateInterval ? SIMTIME_ZERO : iterationTime - updateInterval;
        
        std::cout << "Weight generation time: " << iterationTime << std::endl;
        std::cout << "Scheduling weight update in:" << applyWeightsIn << " seconds" << std::endl;
        
        scheduleAfter(applyWeightsIn, &applyWeightsMsg);
    }
    else if (msg == &applyWeightsMsg){
        applyNewWeights();
        remove(updatePath);
    }
    
}

void DynamicWeights::applyNewWeights() {
    
    using namespace xmlutils;
    ASSERT(dynamicWeights);
    
    const char *topTag = dynamicWeights->getTagName();
    if (!strcmp(topTag, "dynamicWeights")) {
        checkTags(dynamicWeights, "weight");
        
        for (cXMLElement *weight: dynamicWeights->getChildrenByTagName("weight")) {
            // get source libTable reference
            LibTable *srcLibTable = (LibTable *) getModuleByPath(weight->getAttribute("src"))->getModuleByPath(
                    ".libTable");
            std::string weightString = weight->getAttribute("weight");
            double weightDouble = stod(weightString);
            std::string destString = weight->getAttribute("tgt");
            srcLibTable->updateLinkWeight(destString, weightDouble);
        }
        
        return;
    } else if (!strcmp(topTag, "fectables")) {
        for (cXMLElement *fecTable: dynamicWeights->getChildrenByTagName("fectable")) {
            // get source RsvpClassifier reference
            RsvpClassifier *classifier = (RsvpClassifier *) getModuleByPath(
                    fecTable->getAttribute("router"))->getModuleByPath(".classifier");
            classifier->readTableFromXML(fecTable);
        }
        
        return;
    }
}

}