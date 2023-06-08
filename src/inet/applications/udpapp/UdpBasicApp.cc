//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/udpapp/UdpBasicApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpBasicApp);

UdpBasicApp::~UdpBasicApp()
{
    cancelAndDelete(selfMsg);
}

void UdpBasicApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    sendIntervalChangedSignal = registerSignal("sendIntervalChanged");

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        packetName = par("packetName");
        dontFragment = par("dontFragment");
    
        if (stopTime >= CLOCKTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new ClockEvent("sendTimer");
        
        // Check if dynamic demands are being used
        std::string sendIntervalString = par("sendIntervals");
        if (sendIntervalString != ""){
            std::vector<std::string> sendIntervalsStringVector = opp_split(par("sendIntervals"), " ");
            std::vector<std::string> sendIntervalStartTimesStringVector = opp_split(par("sendIntervalStartTimes"), " ");
            // Assert equal lengths
            ASSERT(sendIntervalsStringVector.size() == sendIntervalStartTimesStringVector.size());
    
            for (int i = 0; i < sendIntervalsStringVector.size(); i++){
                double _sendInterval = std::stod(sendIntervalsStringVector[i]);
                double _sendIntervalStartTime = std::stod(sendIntervalStartTimesStringVector[i]);
                sendIntervals.push_back(_sendInterval);
                sendIntervalStartTimes.push_back(_sendIntervalStartTime);
            }
            for (int i = 0; i < sendIntervals.size() - 1; i++){
                double slope = (sendIntervals[i + 1] - sendIntervals[i]) / (sendIntervalStartTimes[i + 1] - sendIntervalStartTimes[i]);
                sendIntervalSlope.push_back(slope);
            }
            
    
            // Assert that start times are in ascending order
            std::vector<double>::iterator pos =  std::adjacent_find (sendIntervalStartTimes.begin(), sendIntervalStartTimes.end(),   // range
                                                                     std::greater<double>());
            if (pos == sendIntervalStartTimes.end())
            {
            }
            else
            {
                throw cRuntimeError("SendIntervalStartTimes are not sorted in ascending order");
            }
            dynamicSendIntervals = true;
        }
    }
    
}

void UdpBasicApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UdpBasicApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
    socket.setCallback(this);
}

L3Address UdpBasicApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void UdpBasicApp::sendPacket()
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    if (dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    emit(packetSentUDPSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}

void UdpBasicApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND);
        if (!dynamicSendIntervals)
            processSend();
        else{
            scheduleClockEventAt(sendIntervalStartTimes[0], selfMsg);
        }
            
            
    }
    else {
        if (stopTime >= CLOCKTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleClockEventAt(stopTime, selfMsg);
        }
    }
}

void UdpBasicApp::processSend()
{
    sendPacket();
    clocktime_t d;
    if(!dynamicSendIntervals)
        d = par("sendInterval");
    else
        d = computeNextSendInterval();
    if (SIMTIME_DBL(d) == 0){
        const char* source = getParentModule()->gate("pppg$o", 0)->getNextGate()->getOwnerModule()->getFullName();
        std::vector<std::string> targetRouters;
        for (std::string tgtAdress : getDestAddressStr())
        {
            std::string tgtRouter = getModuleByPath(tgtAdress.c_str())->gate("pppg$o", 0)->getNextGate()->getOwnerModule()->getFullName();
            targetRouters.push_back(tgtRouter);
        }
    }
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + d < stopTime){
        selfMsg->setKind(SEND);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }
    
    if (d != oldSendInterval)
    {
        emit(sendIntervalChangedSignal, SIMTIME_DBL(d), this);
    }
    oldSendInterval = d;
    
}

void UdpBasicApp::processStop()
{
    socket.close();
}

void UdpBasicApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void UdpBasicApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpBasicApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpBasicApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpBasicApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpBasicApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    emit(packetReceivedUDPSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

void UdpBasicApp::handleStartOperation(LifecycleOperation *operation)
{
    clocktime_t start = std::max(startTime, getClockTime());
    if ((stopTime < CLOCKTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleClockEventAt(start, selfMsg);
    }
}

void UdpBasicApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpBasicApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

clocktime_t UdpBasicApp::computeNextSendInterval()
{
    clocktime_t currentTime = getClockTime();
    if (sendIntervalIndex == sendIntervalStartTimes.size() - 1)
        return sendIntervals[sendIntervalIndex];
    
    double nextStartTime = sendIntervalStartTimes[sendIntervalIndex + 1];
    while (nextStartTime < currentTime.dbl() && sendIntervalIndex < sendIntervalStartTimes.size() - 1){
        sendIntervalIndex++;
        nextStartTime = sendIntervalStartTimes[sendIntervalIndex + 1];
    }
    if (sendIntervalIndex == sendIntervalStartTimes.size() - 1)
        return sendIntervals[sendIntervalIndex];
    
    double currentStartTime = sendIntervalStartTimes[sendIntervalIndex];
    double nextSendInterval = sendIntervals[sendIntervalIndex] + (currentTime.dbl() - currentStartTime) * sendIntervalSlope[sendIntervalIndex];
    double nextSendTime = currentTime.dbl() + nextSendInterval;
    
    if (nextSendTime > nextStartTime){
        sendIntervalIndex += 1;
        return nextStartTime - currentTime.dbl();
    }
    
    return nextSendInterval;
}

} // namespace inet

