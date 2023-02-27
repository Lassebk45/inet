#ifndef DEBUGGING_H_INCLUDED
#define DEBUGGING_H_INCLUDED
// File by Nikolaus Suess for debugging purposes

#include <type_traits>
#include "inet/common/packet/Packet.h"

namespace inet {

#define DEBUG(x) \
    std::cerr << __FILE__ << ":" << __LINE__ << ":" << __func__ << ": " << x << std::endl;

#define IS_OF_TYPE(variable, type, todo) \
        (std::is_same<decltype(variable), type>::value ? variable->todo : "")

#define ROUTER_STR(packet) \
    "[ROUTER " << \
      IS_OF_TYPE(packet, Packet*, getArrivalGate()->getOwner()->getOwner()->getName()) << \
      "]: "

    inline void print_packet_tags(inet::Packet *msg) {
        auto tags = msg->getTags();
        auto num_tags = tags.getNumTags();
        for (int i = 0; i < num_tags; ++i) {
            EV_DEBUG << tags.getTag(i) << endl;
        }
    }
}
#endif
