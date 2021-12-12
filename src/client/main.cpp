#include <iostream>

#include "client.hpp"

int main(int argc, char *argv[]) {
    try {
        Client client(argv[1]);

        while(true) {
            if (client.send() <= 0) {
                return 1;
                continue;
            };
            if (client.recv() <= 0) {
                printf("Stop client\n");
                return 0;
            }
        }
    }
    catch(const char* error) {
        std::cout << error << '\n';
    }
    
	return 0;
}