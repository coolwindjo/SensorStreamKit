#include <zmq.hpp>

int main() {
    // Initialize ZeroMQ context with a single IO thread
    zmq::context_t context(1);

    // 1. Inbound (From Publishers): Bind to 5555
    zmq::socket_t frontend(context, ZMQ_XSUB);
    frontend.bind("tcp://*:5555");      // Publishers connect here

    // 2. Outbound (To Subscribers): Bind to 5556
    zmq::socket_t backend(context, ZMQ_XPUB);
    backend.bind("tcp://*:5556");       // Subscribers connect here

    // 3. Start the proxy to forward messages between frontend and backend
    zmq::proxy(frontend, backend);
    
    return 0;
}