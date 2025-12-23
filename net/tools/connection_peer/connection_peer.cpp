#include <conio.h>
#include <iostream>
#include <thread>

#include "ducklib/net/connection.h"

using namespace ducklib;

bool has_input() {
    return _kbhit();
}

void read_input(std::span<char> buffer, uint32_t& caret) {
    auto ch = _getch();
    
    if (caret >= buffer.size()) {
        return;
    }
    
    buffer[caret++] = ch;
}

int main() {
    auto run = true;
    std::unique_ptr<net::Connection> connection = {};
    
    while (run) {
        while (has_input()) {
            auto ch = _getch();
            
            switch (ch) {
            case '\r': { // Carriage return
                std::string line;
                std::getline(std::cin, line);
                
                // Process input
                if (line.starts_with('/')) {
                    if (line == "/q" || line == "/quit") {
                        run = false;
                        break;
                    } else if (line.starts_with("/connect ")) {
                        // TODO: Parse address
                        auto address = line.substr(line.find(' ') + 1);
                        std::cout << "\033[A\033[K" << "Connecting to: " << address << "...\n";
                        auto port = 20020;
                        auto address_no_port = address;
                        
                        if (address.contains(':')) {
                            auto port_index = address.find(':');
                            auto port_str = address.substr(port_index + 1);
                            std::from_chars(port_str.data(), port_str.data() + port_str.size(), port);
                            address_no_port = address.substr(0, port_index);
                            std::cout << port << std::endl;
                            std::cout << address_no_port << std::endl;
                        }
                        
                        connection = std::make_unique<net::Connection>(address_no_port, port);
                    }
                } else {
                    // TODO: Send over connection
                    std::cout << "\033[A\033[K" << "You: " << line << std::endl;
                }
            }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}