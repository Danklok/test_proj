#include <iostream>

#include "client.hpp"

int main(int argc, char *argv[]) {
    try {
        Client client(argv[1]);

        while(true) {
            if (client.send() <= 0) {
                continue;
            };
            client.recv();
        }
    }
    catch(const char* error) {
        std::cout << error << '\n';
    }
    
	return 0;
}