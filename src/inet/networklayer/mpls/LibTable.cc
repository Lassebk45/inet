//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/LibTable.h"

#include <iostream>
#include <algorithm>

#include "inet/common/XMLUtils.h"

#include "inet/debugging.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/NetworkInterface.h"

#include "inet/p10/LibTableUpdate_m.h"

namespace inet {

Define_Module(LibTable);

void LibTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        maxLabel = 0;
        WATCH_VECTOR(lib);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        libTableChangedSignal = registerSignal("libTableChanged");
        readTableFromXML(par("config"));
    }

}

void LibTable::handleMessage(cMessage * msg)
{
    using namespace xmlutils;
    
    if (dynamic_cast<LibTableUpdate*>(msg)){
        LibTableUpdate* updateMessage = (LibTableUpdate*) msg;
        const cXMLElement *element = updateMessage->getUpdateElement();
        const char* updateType = element->getTagName();
        if (strcmp(updateType, "addLibEntry") == 0)
        {
            // Check that all subelements are present
            checkTags(element, "priority inLabel inInterface outInterface outLabel");
            
            // Get the label stack operations
            LabelOpVector opsVector;
            cXMLElement* outLabelElement = element->getFirstChildWithTag("outLabel");
            checkTags(outLabelElement, "op");
            cXMLElementList ops = outLabelElement->getChildren();
            for (auto& ops_oit : ops) {
                const cXMLElement& op = *ops_oit;
                const char *val = op.getAttribute("value");
                const char *code = op.getAttribute("code");
                ASSERT(code);
                LabelOp l;
        
                if (!strcmp(code, "push")) {
                    l.optcode = PUSH_OPER;
                    ASSERT(val);
                    l.label = atoi(val);
                    ASSERT(l.label > 0);
                }
                else if (!strcmp(code, "pop")) {
                    l.optcode = POP_OPER;
                    ASSERT(!val);
                }
                else if (!strcmp(code, "swap")) {
                    l.optcode = SWAP_OPER;
                    ASSERT(val);
                    l.label = atoi(val);
                    ASSERT(l.label > 0);
                }
                else
                    ASSERT(false);
        
                opsVector.push_back(l);
            }
            
            
            installLibEntry(getParameterIntValue(element, "inLabel"), getParameterStrValue(element, "inInterface"), opsVector, getParameterStrValue(element, "outInterface"), 1, getParameterIntValue(element, "priority"));
        }
        /*if (FILE *file = fopen(updatePath.c_str(), "r")) {
            fclose(file);
            updateTableFromXML(getEnvir()->getXMLDocument(updatePath.c_str()));
            remove(updatePath.c_str());
        }*/
        delete msg;
    }
}

void LibTable::updateLibTable(cXMLElement *updateElement){
    using namespace xmlutils;
    const char* updateType = updateElement->getTagName();
    if (strcmp(updateType, "add") == 0)
    {
        checkTags(updateElement, "priority inLabel inRouter outRouter outLabel");
        LabelOpVector opsVector;
        cXMLElement* outLabelElement = updateElement->getFirstChildWithTag("outLabel");
        checkTags(outLabelElement, "op");
        cXMLElementList ops = outLabelElement->getChildren();
        for (auto& ops_oit : ops) {
            const cXMLElement& op = *ops_oit;
            const char *val = op.getAttribute("value");
            const char *code = op.getAttribute("code");
            ASSERT(code);
            LabelOp l;
        
            if (!strcmp(code, "push")) {
                l.optcode = PUSH_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else if (!strcmp(code, "pop")) {
                l.optcode = POP_OPER;
                ASSERT(!val);
            }
            else if (!strcmp(code, "swap")) {
                l.optcode = SWAP_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else
                ASSERT(false);
        
            opsVector.push_back(l);
        }
        const char* sourceRouter = updateElement->getAttribute("router");
        const char* targetRouter = getParameterStrValue(updateElement, "outRouter");
        const char* inRouter = getParameterStrValue(updateElement, "inRouter");
    
    
        // Find the outgoing interface
        std::string outInterface = "";
        if (strcmp(sourceRouter, targetRouter) == 0)
        {
            outInterface = "mlo0";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(targetRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    outInterface = "ppp" + std::to_string(i);
                    break;
                }
            }
            if (strcmp(outInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find the outInterface connecting router %s to router %s", sourceRouter, targetRouter);
            }
        }
        //
        // Get the inInterface
        std::string inInterface = "";
        if (strcmp(inRouter, "any") == 0)
        {
            inInterface = "any";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(inRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    //printf("inRouter: %s\n", this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName());
    
                    inInterface = "ppp" + std::to_string(i);
                    //printf("inInterface: %s\n", inInterface.c_str());
                    break;
                }
            }
            if (strcmp(inInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find inInterface connecting router %s to router %s", inRouter, sourceRouter);
            }
        }
        
        // Add the rule
        installLibEntry(getParameterIntValue(updateElement, "inLabel"), inInterface.c_str(), opsVector, outInterface.c_str(), 1, getParameterIntValue(updateElement, "priority"));
    }
    else if (strcmp(updateType, "remove") == 0)
    {
        checkTags(updateElement, "priority inLabel inRouter");
        const char* sourceRouter = updateElement->getAttribute("router");
        const char* inRouter = getParameterStrValue(updateElement, "inRouter");
        // Convert inRouter to inInterface
        std::string inInterface = "";
        if (strcmp(inRouter, "any") == 0)
        {
            inInterface = "any";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(inRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    //printf("inRouter: %s\n", this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName());

                    inInterface = "ppp" + std::to_string(i);
                    //printf("inInterface: %s\n", inInterface.c_str());
                    break;
                }
            }
            if (strcmp(inInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find inInterface connecting router %s to router %s", inRouter, sourceRouter);
            }
        }

        removeLibEntry(getParameterIntValue(updateElement, "inLabel"), inInterface.c_str(), getParameterIntValue(updateElement, "priority"));

    }
    else if (strcmp(updateType, "swap") == 0)
    {
        checkTags(updateElement, "priority inLabel inRouter outRouter outLabel");
        const char* sourceRouter = updateElement->getAttribute("router");
        const char* inRouter = getParameterStrValue(updateElement, "inRouter");
        // Convert inRouter to inInterface
        std::string inInterface = "";
        if (strcmp(inRouter, "any") == 0)
        {
            inInterface = "any";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(inRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    //printf("inRouter: %s\n", this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName());

                    inInterface = "ppp" + std::to_string(i);
                    //printf("inInterface: %s\n", inInterface.c_str());
                    break;
                }
            }
            if (strcmp(inInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find inInterface connecting router %s to router %s", inRouter, sourceRouter);
            }
        }

        removeLibEntry(getParameterIntValue(updateElement, "inLabel"), inInterface.c_str(), getParameterIntValue(updateElement, "priority"));

        LabelOpVector opsVector;
        cXMLElement* outLabelElement = updateElement->getFirstChildWithTag("outLabel");
        checkTags(outLabelElement, "op");
        cXMLElementList ops = outLabelElement->getChildren();
        for (auto& ops_oit : ops) {
            const cXMLElement& op = *ops_oit;
            const char *val = op.getAttribute("value");
            const char *code = op.getAttribute("code");
            ASSERT(code);
            LabelOp l;

            if (!strcmp(code, "push")) {
                l.optcode = PUSH_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else if (!strcmp(code, "pop")) {
                l.optcode = POP_OPER;
                ASSERT(!val);
            }
            else if (!strcmp(code, "swap")) {
                l.optcode = SWAP_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else
                ASSERT(false);

            opsVector.push_back(l);
        }
        const char* targetRouter = getParameterStrValue(updateElement, "outRouter");


        // Find the outgoing interface
        std::string outInterface = "";
        if (strcmp(sourceRouter, targetRouter) == 0)
        {
            outInterface = "mlo0";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(targetRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    outInterface = "ppp" + std::to_string(i);
                    break;
                }
            }
            if (strcmp(outInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find the outInterface connecting router %s to router %s", sourceRouter, targetRouter);
            }
        }
        //
        // Get the inInterface
        inInterface = "";
        if (strcmp(inRouter, "any") == 0)
        {
            inInterface = "any";
        }
        else
        {
            for (int i = 0; i < this->getParentModule()->gateSize("pppg$o"); i++)
            {
                if (strcmp(inRouter, this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName()) == 0)
                {
                    //printf("inRouter: %s\n", this->getParentModule()->gate("pppg$o", i)->getNextGate()->getOwnerModule()->getFullName());

                    inInterface = "ppp" + std::to_string(i);
                    //printf("inInterface: %s\n", inInterface.c_str());
                    break;
                }
            }
            if (strcmp(inInterface.c_str(), "") == 0){
                throw cRuntimeError("Cannot find inInterface connecting router %s to router %s", inRouter, sourceRouter);
            }
        }

        // Add the rule
        installLibEntry(getParameterIntValue(updateElement, "inLabel"), inInterface.c_str(), opsVector, outInterface.c_str(), 1, getParameterIntValue(updateElement, "priority"));
    }
    else
    {
        throw cRuntimeError("Invalid updateType: %s\n", updateType);
    }
}

/**
 * Checks if the interface of the forwarding entry is up (TRUE) or down (FALSE).
 */
bool LibTable::isInterfaceUp(const std::string& ifname){
    const cModule* router = this->getParentModule();
    const InterfaceTable* ift = (InterfaceTable*)CHK(router->getModuleByPath(".interfaceTable"));
    EV_DEBUG << "INTERFACE TABLE - # interfaces: " << ift->getNumInterfaces() << endl;
    int nr_interfaces = ift->getNumInterfaces();
    for( int i = 0; i < nr_interfaces; ++i ){
        const NetworkInterface* interface = ift->getInterface(i);
        EV_DEBUG << "Name: " << interface->getInterfaceName() << " ";
        EV_DEBUG << interface->getInterfaceId() << " status = " << interface->isUp() << endl;
        if( !strcmp(interface->getInterfaceName(), ifname.c_str() ) ){
            return interface->isUp();
        }
    }

    return false;

}

/**
 * Get entry from LIB table.
 * Modified.
 */
bool LibTable::resolveLabel(std::string inInterface, int inLabel,
        LabelOpVector& outLabel, std::string& outInterface, int& color)
{
    EV_ERROR << "In resolve label ... inInterface: " << inInterface << endl;

    bool any = (inInterface.length() == 0);
    any = true; // TODO: fix


    for (auto& elem : lib) {
        if (!any && elem.inInterface != inInterface)
            continue;

        if (elem.inLabel != inLabel)
            continue;

        EV_DEBUG << "Resolving label from " << std::to_string(inLabel) << " to " << endl;
        for(const auto& fwe : elem.entries){
            EV_ERROR << "- Entry with" << endl;
            EV_ERROR << "TEST" << endl;
            EV_ERROR << "  out_if:   " << fwe.outInterface << endl;
            EV_ERROR << "  priority: " << fwe.priority << endl;
            for( const auto& e : fwe.outLabel)
                EV_ERROR << "   * " << std::to_string(e.optcode) << " "<< std::to_string(e.label) << endl;
        }

        // Filter interfaces to get only those that are up.
        std::vector<ForwardingEntry> valid_entries;
        std::copy_if(elem.entries.begin(), elem.entries.end(), std::back_inserter(valid_entries), [this](const auto&e){
            return this->isInterfaceUp(e.outInterface);
        });

        //outLabel = elem.outLabel;
        //outInterface = elem.outInterface;
        // Find entry with lowest priority
        auto it = std::min_element(valid_entries.begin(), valid_entries.end(), [](const auto& e1, const auto& e2){
            return e1.priority < e2.priority;
        });

        if( it == valid_entries.end())
            return false;

        // Implementation of ECMP -- currently just a proof of concept.
        // NOTE: Very experimental code ...
        int min_priority = it->priority;
        std::vector<ForwardingEntry> minimum_entries;
        std::copy_if(valid_entries.begin(), valid_entries.end(), std::back_inserter(minimum_entries), [min_priority](const auto&e){
           return e.priority == min_priority;
        });

        // Note: Not the best way to do it, just proof of concept ...
        it = minimum_entries.begin();
        std::advance( it, std::rand() % minimum_entries.size() );
        // END ECMP CODE

        outLabel = it->outLabel;
        outInterface = it->outInterface;
        EV_ERROR << "Using ("<<outLabel <<","<<outInterface<<")"<< endl;

        color = elem.color;

        return true;
    }
    return false;
}

// NOTE: Modified.
// Now, it does not overwrite an entry if it already exists but instead it adds an additional one.
int LibTable::installLibEntry(int inLabel, std::string inInterface, const LabelOpVector& outLabel,
        std::string outInterface, int color, int priority /* = 0 */)
{
    for (auto& elem : lib) {
        //if (elem.inLabel != inLabel)
        if (elem.inLabel != inLabel /* || elem.inInterface != inInterface ???*/)
            continue;
        //elem.inInterface = inInterface;
        //elem.outLabel = outLabel;
        //elem.outInterface = outInterface;
        ForwardingEntry fwe { outLabel, outInterface, priority };

        elem.entries.push_back(fwe);
        elem.color = color;
        emit(libTableChangedSignal, this);
        return inLabel;
    }
    LibEntry newItem;
    newItem.inLabel = inLabel;
    newItem.inInterface = inInterface;

    ForwardingEntry fwe { outLabel, outInterface, priority };
    newItem.entries.push_back(fwe);
    newItem.color = color;
    lib.push_back(newItem);
    emit(libTableChangedSignal, this);
    return inLabel;
}

int LibTable::swapLibEntry(int inLabel, std::string inInterface, const LabelOpVector& outLabel,
                              std::string outInterface, int color, int priority /* = 0 */)
{
    int elemFound = 0;
    for (auto& elem : lib) {
        //if (elem.inLabel != inLabel)
        if (elem.inLabel != inLabel /* || elem.inInterface != inInterface ???*/)
            continue;
        
        //elem.inInterface = inInterface;
        //elem.outLabel = outLabel;
        //elem.outInterface = outInterface;
        elemFound = 1;
        ForwardingEntry fwe { outLabel, outInterface, priority };
        int index = -1;
        int priorityFound = 0;
        for (int i; i < elem.entries.size(); i++)
        {
            if (elem.entries[i].priority == priority)
            {
                index = i;
                break;
            }
        }
        if (index != -1){
            elem.entries[index] = fwe;
        }
        else
        {
            throw cRuntimeError("Could not swap forwarding entry rule on router %s with label %d, inInterface %s and priority %d. No rule with priority %d currently exists \n", this->getParentModule()->getFullName(), inLabel, inInterface.c_str(), priority, priority);
        }
        elem.color = color;
        emit(libTableChangedSignal, this);
        return inLabel;
    }
    
    if (!elemFound)
    {
        throw cRuntimeError("Could not swap forwarding entry rule on router %s with label %d, inInterface %s and priority %d. No rule with label %d currently exists \n", this->getParentModule()->getFullName(), inLabel, inInterface.c_str(), priority, inLabel);
    }
    ASSERT(false);
    emit(libTableChangedSignal, this);
    return 0; // prevent warning
}

int LibTable::removeLibEntry(int inLabel, std::string inInterface, int priority)
{
    for (unsigned int i = 0; i < lib.size(); i++) {
        if (lib[i].inLabel != inLabel)
            continue;

        for (unsigned int j = 0; j < lib[i].entries.size(); j++)
        {
            if (lib[i].entries[j].priority != priority)
                continue;
            if(lib[i].entries.size() < 2)
                lib.erase(lib.begin() + i);
            else
                lib[i].entries.erase(lib[i].entries.begin() + j);

            emit(libTableChangedSignal, this);
            return 0;
        }
    }
    return 0;
}

void LibTable::removeLibEntry(int inLabel)
{
    for (unsigned int i = 0; i < lib.size(); i++) {
        if (lib[i].inLabel != inLabel)
            continue;

        lib.erase(lib.begin() + i);
        emit(libTableChangedSignal, this);
        return;
    }
    ASSERT(false);
}

void LibTable::readTableFromXML(const cXMLElement *libtable)
{
    using namespace xmlutils;

    ASSERT(libtable);
    ASSERT(!strcmp(libtable->getTagName(), "libtable"));
    checkTags(libtable, "libentry");
    cXMLElementList list = libtable->getChildrenByTagName("libentry");
    for (auto& elem : list) {
        const cXMLElement& entry = *elem;

        checkTags(&entry, "inLabel inInterface outLabel outInterface color priority");

        LibEntry newItem;
        newItem.inLabel = getParameterIntValue(&entry, "inLabel");
        newItem.inInterface = getParameterStrValue(&entry, "inInterface");
        newItem.color = getParameterIntValue(&entry, "color", 0);

        ForwardingEntry fwe {
            {}, // LabelOpVector outLabel
            getParameterStrValue(&entry, "outInterface"),
            getParameterIntValue(&entry, "priority", 0)
        };

        cXMLElementList ops = getUniqueChild(&entry, "outLabel")->getChildrenByTagName("op");
        for (auto& ops_oit : ops) {
            const cXMLElement& op = *ops_oit;
            const char *val = op.getAttribute("value");
            const char *code = op.getAttribute("code");
            ASSERT(code);
            LabelOp l;

            if (!strcmp(code, "push")) {
                l.optcode = PUSH_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else if (!strcmp(code, "pop")) {
                l.optcode = POP_OPER;
                ASSERT(!val);
            }
            else if (!strcmp(code, "swap")) {
                l.optcode = SWAP_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else
                ASSERT(false);

            fwe.outLabel.push_back(l);
        }

        auto old_entry = std::find_if(lib.begin(), lib.end(), [&newItem](const LibEntry& entry){
            return entry == newItem;
        });

        if( old_entry == lib.end() ){
            // There is no entry, yet.
            newItem.entries.push_back(fwe);
            lib.push_back(newItem);
        }
        else {
            // LIB entry already exists, we just add a new forwarding rule.
            old_entry->entries.push_back(fwe);
        }

        ASSERT(newItem.inLabel > 0);

        if (newItem.inLabel > maxLabel)
            maxLabel = newItem.inLabel;
    }

    emit(libTableChangedSignal, this);
}

LabelOpVector LibTable::pushLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = PUSH_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LibTable::swapLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = SWAP_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LibTable::popLabel()
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = POP_OPER;
    lop.label = 0;
    vec.push_back(lop);
    return vec;
}

/**
 * Compare by inLabel and inInterface.
 */
bool operator==(const LibTable::LibEntry& lhs, const LibTable::LibEntry& rhs){
    return lhs.inLabel == rhs.inLabel && lhs.inInterface == rhs.inInterface;
}
bool operator!=(const LibTable::LibEntry& lhs, const LibTable::LibEntry& rhs){
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const LabelOpVector& label)
{
    os << "{";
    for (unsigned int i = 0; i < label.size(); i++) {
        switch (label[i].optcode) {
            case PUSH_OPER:
                os << "PUSH " << label[i].label;
                break;

            case SWAP_OPER:
                os << "SWAP " << label[i].label;
                break;

            case POP_OPER:
                os << "POP";
                break;

            default:
                ASSERT(false);
                break;
        }

        if (i < label.size() - 1)
            os << "; ";
        else
            os << "}";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const LibTable::LibEntry& lib)
{
    os << "inLabel:" << lib.inLabel;
    os << "    inInterface:" << lib.inInterface;
    //os << "    outLabel:" << lib.outLabel;
    //os << "    outInterface:" << lib.outInterface;
    os << "    color:" << lib.color;
    os << "    entries: [";
    for( const auto& e : lib.entries){
        os << "[";
        os << "    outLabel:" << e.outLabel;
        os << "    outInterface:" << e.outInterface;
        os << "    priority:" << e.priority;
        os << "],";
    }
    os << "    ]";
    return os;
}
} // namespace inet

