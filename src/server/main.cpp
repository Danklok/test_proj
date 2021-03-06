#include <iostream>

#include "server.hpp"

int main(void) {
    try {
        sockaddr_in client_addr;
        uint32_t crc32 = 0;

        server srv;
        while(true) {
            size_t recv_size = srv.recv();
            if (recv_size <= 0) {
                continue;
            }
            size_t msg_size = srv.construct_complete(recv_size, crc32);
            srv.send(msg_size);
        }
    }
    catch(const char* error) {
        std::cout << error << '\n';
    }
    
	return 0;
}