#include <iostream>
#include "sensorstreamkit/transport/zmq_transport.hpp"

int main() {

    sensorstreamkit::transport::ZmqTransport transport;
    
    std::cout << "Broker starting..." << std::endl;
    std::cout << "Frontend (Publishers): tcp://*:5555" << std::endl;
    std::cout << "Backend (Subscribers): tcp://*:5556" << std::endl;

    // 1. Inbound (From Publishers): Bind to 5555
    // 2. Outbound (To Subscribers): Bind to 5556
    // 3. Start the proxy to forward messages between frontend and backend
    transport.run_broker("tcp://*:5555", "tcp://*:5556");
    
    return 0;
}