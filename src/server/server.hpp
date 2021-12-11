#ifndef _SERVER_H_
#define _SERVER_H_

#include <cstddef>
#include <cstdint>
#include <map>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../proto.hpp"

const uint16_t PORT = 6543;
const char     IP[] = "127.0.0.1";

struct Mempool {
    size_t m_recv_packs = 0;
    size_t m_total_packs = 0;
    bool   m_finished = false;
    uint8_t* m_data = nullptr;
    
    Mempool() {
        m_data = new uint8_t[m_total_packs * DATA_SIZE];
    } 

    ~Mempool(){
        delete[] m_data;
    } 
};

class server {
    const char* m_IP;
    const uint16_t m_PORT;
    sockaddr_in m_this_addr;
    int m_sock_fd;
    uint8_t m_buff[UDP_MAX_SIZE];

    std::map<uint64_t, Mempool> m_stats;
public:
    server();
    ~server();

    size_t recv(sockaddr_in& other_addr);
    size_t send(const sockaddr_in& other_addr, size_t msg_len);

    size_t construct_complete(size_t recv_len, uint32_t& crc32);
};

#endif