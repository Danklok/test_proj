#ifndef _SERVER_H_
#define _SERVER_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <map>
#include <set>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../proto.hpp"

const uint16_t PORT = 6543;
const char     IP[] = "127.0.0.1";

struct Mempool {
    size_t m_recv_packs = 0;
    size_t m_total_packs = 0;
    size_t m_recv_bytes = 0;
    bool   m_finished = false;
    uint8_t* m_data = nullptr;
    sockaddr_in client_addr;
    std::set<size_t> m_recv_file_parts;
    
    Mempool(size_t total_packs) : m_total_packs(total_packs) {
        m_data = new uint8_t[m_total_packs * DATA_SIZE];
    } 

    ~Mempool(){
        if (m_data) {
            delete[] m_data;
            m_recv_file_parts.clear();
        }
    } 
};

class server {
    const char* m_IP;
    const uint16_t m_PORT;
    sockaddr_in m_client_addr;
    int m_sock_fd;
    uint8_t m_buff[UDP_MAX_SIZE];
    std::map<uint64_t, std::shared_ptr<Mempool>> m_stats;

    bool check_hash();
public:
    server();
    ~server();

    size_t recv();
    size_t send(size_t msg_len);

    size_t construct_complete(size_t recv_len, uint32_t& crc32);
};

#endif