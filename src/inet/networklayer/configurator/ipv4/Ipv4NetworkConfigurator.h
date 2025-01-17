//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4NETWORKCONFIGURATOR_H
#define __INET_IPV4NETWORKCONFIGURATOR_H

#include <algorithm>

#include "inet/common/Topology.h"
#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

/**
 * This module provides the global static configuration for the Ipv4RoutingTable and
 * the Ipv4 network interfaces of all nodes in the network.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv4NetworkConfigurator : public L3NetworkConfiguratorBase
{
  protected:
    /**
     * Represents a node in the network.
     */
    class INET_API Node : public L3NetworkConfiguratorBase::Node {
      public:
        std::vector<Ipv4Route *> staticRoutes;
        std::vector<Ipv4MulticastRoute *> staticMulticastRoutes;

      public:
        Node(cModule *module) : L3NetworkConfiguratorBase::Node(module) {}
        ~Node() {
            for (size_t i = 0; i < staticRoutes.size(); i++) delete staticRoutes[i];
            for (size_t i = 0; i < staticMulticastRoutes.size(); i++) delete staticMulticastRoutes[i];
        }
    };

    class INET_API Topology : public L3NetworkConfiguratorBase::Topology {
      protected:
        virtual Node *createNode(cModule *module) override { return new Ipv4NetworkConfigurator::Node(module); }
    };

    /**
     * Represents an interface in the network.
     */
    class INET_API InterfaceInfo : public L3NetworkConfiguratorBase::InterfaceInfo {
      public:
        uint32_t address; // the bits
        uint32_t addressSpecifiedBits; // 1 means the bit is specified, 0 means the bit is unspecified
        uint32_t netmask; // the bits
        uint32_t netmaskSpecifiedBits; // 1 means the bit is specified, 0 means the bit is unspecified
        std::vector<Ipv4Address> multicastGroups;

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface);

        Ipv4Address getAddress() const { ASSERT(addressSpecifiedBits == 0xFFFFFFFF); return Ipv4Address(address); }
        Ipv4Address getNetmask() const { ASSERT(netmaskSpecifiedBits == 0xFFFFFFFF); return Ipv4Address(netmask); }
    };

    /**
     * Simplified route representation used by the optimizer.
     * This class makes the optimization faster by introducing route coloring.
     */
    class INET_API RouteInfo {
      public:
        int color; // an index into an array representing the different route actions (gateway, interface, metric, etc.)
        bool enabled; // allows turning of routes without removing them from the list
        uint32_t destination; // originally copied from the Ipv4Route
        uint32_t netmask; // originally copied from the Ipv4Route
        std::vector<RouteInfo *> originalRouteInfos; // routes that are routed by this one from the unoptimized original routing table, we keep track of this to be able to skip merge candidates with less computation

      public:
        RouteInfo(int color, uint32_t destination, uint32_t netmask) { this->color = color; this->enabled = true; this->destination = destination; this->netmask = netmask; }
        ~RouteInfo() {} // don't delete originalRouteInfos elements, they are not exclusively owned

        std::string str() const {
            std::stringstream out;
            out << "color = " << color << ", destination = " << Ipv4Address(destination) << ", netmask = " << Ipv4Address(netmask);
            return out.str();
        }
    };

    /**
     * Simplified routing table representation used by the optimizer.
     */
    class INET_API RoutingTableInfo {
      public:
        std::vector<RouteInfo *> routeInfos; // list of routes in the routing table

      public:
        RoutingTableInfo() {}
        ~RoutingTableInfo() {}

        int addRouteInfo(RouteInfo *routeInfo);
        void removeRouteInfo(const RouteInfo *routeInfo) { routeInfos.erase(find(routeInfos.begin(), routeInfos.end(), routeInfo)); }
        RouteInfo *findBestMatchingRouteInfo(const uint32_t destination, int begin = 0, int end = -1) const { return findBestMatchingRouteInfo(routeInfos, destination, 0, end == -1 ? routeInfos.size() : end); }
        static RouteInfo *findBestMatchingRouteInfo(const std::vector<RouteInfo *>& routeInfos, const uint32_t destination, int begin, int end);
        static bool routeInfoLessThan(const RouteInfo *a, const RouteInfo *b) { return a->netmask != b->netmask ? a->netmask > b->netmask : a->destination < b->destination; }
    };

  protected:
    // parameters
    bool assignAddressesParameter = false;
    bool assignUniqueAddresses = false;
    bool assignDisjunctSubnetAddressesParameter = false;
    bool addStaticRoutesParameter = false;
    bool addSubnetRoutesParameter = false;
    bool addDefaultRoutesParameter = false;
    bool addDirectRoutesParameter = false;
    bool optimizeRoutesParameter = false;

    // internal state
    Topology topology;

  public:
    /**
     * Computes the Ipv4 network configuration for all nodes in the network.
     * The result of the computation is only stored in the network configurator.
     */
    virtual void computeConfiguration();
    
    virtual simtime_t networkFlushTime(double flushConstant){return topology.getDiameter() * flushConstant / 1000;};

    /**
     * Configures all interfaces in the network based on the current network configuration.
     */
    virtual void configureAllInterfaces();

    /**
     * Configures the provided interface based on the current network configuration.
     */
    virtual void configureInterface(NetworkInterface *networkInterface);

    /**
     * Configures all routing tables in the network based on the current network configuration.
     */
    virtual void configureAllRoutingTables();

    /**
     * Configures the provided routing table based on the current network configuration.
     */
    virtual void configureRoutingTable(IIpv4RoutingTable *routingTable);

    /**
     * Configures the provided routing table based on the current network configuration for specified networkInterface.
     */
    virtual void configureRoutingTable(IIpv4RoutingTable *routingTable, NetworkInterface *networkInterface);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;

    /**
     * Reads interface elements from the configuration file and stores result.
     */
    virtual void readInterfaceConfiguration(Topology& topology);

    /**
     * Reads multicast-group elements from the configuration file and stores the result
     */
    virtual void readMulticastGroupConfiguration(Topology& topology);

    /**
     * Reads route elements from configuration file and stores the result
     */
    virtual void readManualRouteConfiguration(Topology& topology);

    /**
     * Reads multicast-route elements from configuration file and stores the result.
     */
    virtual void readManualMulticastRouteConfiguration(Topology& topology);

    /**
     * Assigns the addresses for all interfaces based on the parameters given
     * in the configuration file. See the NED file for details.
     */
    virtual void assignAddresses(Topology& topology);

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     * May add default routes and subnet routes if possible and requested.
     */
    virtual void addStaticRoutes(Topology& topology, cXMLElement *element);

    /**
     * Destructively optimizes the given Ipv4 routes by merging some of them.
     * The resulting routes might be different in that they will route packets
     * that the original routes did not. Nevertheless the following invariant
     * holds: any packet routed by the original routes will still be routed
     * the same way by the optimized routes.
     */
    virtual void optimizeRoutes(std::vector<Ipv4Route *>& routes);

    void ensureConfigurationComputed(Topology& topology);
    void configureInterface(InterfaceInfo *interfaceInfo);
    void configureRoutingTable(Node *node);
    void configureRoutingTable(Node *node, NetworkInterface *networkInterface);

    /**
     * Prints the current network configuration to the module output.
     */
    virtual void dumpConfiguration();
    virtual void dumpLinks(Topology& topology);
    virtual void dumpAddresses(Topology& topology);
    virtual void dumpRoutes(Topology& topology);
    virtual void dumpConfig(Topology& topology);

    // helper functions
    virtual InterfaceInfo *createInterfaceInfo(L3NetworkConfiguratorBase::Topology& topology, L3NetworkConfiguratorBase::Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface) override;
    virtual void parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits);
    virtual bool linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule);
    virtual void resolveInterfaceAndGateway(Node *node, const char *interfaceAttr, const char *gatewayAttr, NetworkInterface *& outIE, Ipv4Address& outGateway, Topology& topology);
    virtual InterfaceInfo *findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node);
    virtual InterfaceInfo *findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, Ipv4Address address);
    virtual LinkInfo *findLinkOfInterface(Topology& topology, NetworkInterface *networkInterface);
    virtual IRoutingTable *findRoutingTable(L3NetworkConfiguratorBase::Node *node) override;
    virtual void assignAddresses(std::vector<LinkInfo *> links);

    // helpers for address assignment
    static bool compareInterfaceInfos(InterfaceInfo *i, InterfaceInfo *j);
    void collectCompatibleInterfaces(const std::vector<InterfaceInfo *>& interfaces, /*in*/
            std::vector<InterfaceInfo *>& compatibleInterfaces, /*out, and the rest too*/
            uint32_t& mergedAddress, uint32_t& mergedAddressSpecifiedBits, uint32_t& mergedAddressIncompatibleBits,
            uint32_t& mergedNetmask, uint32_t& mergedNetmaskSpecifiedBits, uint32_t& mergedNetmaskIncompatibleBits);

    // helpers for routing table optimization
    bool containsRoute(const std::vector<Ipv4Route *>& routes, Ipv4Route *route);
    bool routesHaveSameColor(Ipv4Route *route1, Ipv4Route *route2);
    int findRouteIndexWithSameColor(const std::vector<Ipv4Route *>& routes, Ipv4Route *route);
    bool routesCanBeSwapped(RouteInfo *routeInfo1, RouteInfo *routeInfo2);
    bool routesCanBeNeighbors(const std::vector<RouteInfo *>& routeInfos, int i, int j);
    bool interruptsOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, RouteInfo *originalRouteInfo);
    bool interruptsAnyOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos);
    bool interruptsSubsequentOriginalRoutes(const RoutingTableInfo& routingTableInfo, int index);
    void checkOriginalRoutes(const RoutingTableInfo& routingTableInfo, const RoutingTableInfo& originalRoutingTableInfo);
    void findLongestCommonDestinationPrefix(uint32_t destination1, uint32_t netmask1, uint32_t destination2, uint32_t netmask2, uint32_t& destinationOut, uint32_t& netmaskOut);
    void addOriginalRouteInfos(RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos);
    bool tryToMergeTwoRoutes(RoutingTableInfo& routingTableInfo, int i, int j, RouteInfo *routeInfoI, RouteInfo *routeInfoJ);
    bool tryToMergeAnyTwoRoutes(RoutingTableInfo& routingTableInfo);

    // address resolver interface
    bool getInterfaceIpv4Address(L3Address& ret, NetworkInterface *networkInterface, bool netmask) override;
};

} // namespace inet

#endif

