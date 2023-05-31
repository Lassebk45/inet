#include "inet/p10/DynamicWeights.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/common/XMLUtils.h"
#include <chrono>
#include <thread>

namespace inet {

Define_Module(DynamicWeights);

void DynamicWeights::initialize(){
    updatePath = par("initialPath");
    nextUpdateTime = SIMTIME_ZERO;
    
    // Set the initial weights
    scheduleAt(nextUpdateTime, &updateTrigger);
}

void DynamicWeights::handleMessage(cMessage* msg){
    getEnvir()->forgetXMLDocument(updatePath);
    
    FILE *file;
    // When the update file exists load the updates
    std::cout << "Waiting for dynamic weights file..." << std::endl;
    while (!(( access( updatePath, F_OK ) != -1 )))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "Dynamic weights file received." << std::endl;
    const cXMLElement * dynamicWeights = getEnvir()->getXMLDocument(updatePath);
    updatePath = par("updatePath");
    
    applyNewWeights(dynamicWeights);
    simtime_t updateInterval = par("updateInterval");
    if (!initialWeights)
        remove(updatePath);
    else
        initialWeights = false;
    nextUpdateTime += par("updateInterval");
    scheduleAt(nextUpdateTime, &updateTrigger);
}

void DynamicWeights::applyNewWeights(const cXMLElement *dynamicWeights){
    
    using namespace xmlutils;
    ASSERT(dynamicWeights);
    ASSERT(!strcmp(dynamicWeights->getTagName(), "dynamicWeights"));
    
    checkTags(dynamicWeights, "weight");
    
    for(cXMLElement* weight : dynamicWeights->getChildrenByTagName("weight"))
    {
        // get source libTable reference
        LibTable* srcLibTable = (LibTable *)getModuleByPath(weight->getAttribute("src"))->getModuleByPath(".libTable");
        std::string weightString = weight->getAttribute("weight");
        double weightDouble = stod(weightString);
        std::string destString = weight->getAttribute("tgt");
        srcLibTable->updateLinkWeight(destString, weightDouble);
    }
}

}