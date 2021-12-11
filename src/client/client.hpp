#ifndef _SERVER_H_
#define _SERVER_H_

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>
#include <random>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "../proto.hpp"
#include "../crc32.hpp"

struct File {
    int m_file_fd;
    size_t m_file_size = 0;
    size_t m_total_packs = 0;
    size_t m_last_pack_size = 0;
    uint8_t* m_filebuff = nullptr; 
    uint8_t m_ID[ID_SIZE];
    uint32_t m_crc32;
    std::vector<size_t> m_seq_numbers;
    
    File(const std::string& filename);
    ~File();
};

class Client {
    const char* m_IP;
    const uint16_t m_PORT;
    sockaddr_in m_server_addr;
    int m_sock_fd;
    bool m_ticket_recied = false;
    uint8_t m_buff[UDP_MAX_SIZE];
    std::shared_ptr<File> files[16];
public:
    Client(const std::string& filename);
    ~Client();

    size_t recv();
    size_t send();
};

#endif