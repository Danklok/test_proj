#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "server.hpp"
#include "../crc32.hpp"

server::server() : m_IP(IP), m_PORT(PORT) {
	if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		throw "Socket not created";
	}
	
	memset((char*)&m_this_addr, 0, sizeof(m_this_addr));
	
	m_this_addr.sin_family = AF_INET;
	m_this_addr.sin_port = htons(m_PORT);
	m_this_addr.sin_addr.s_addr = htonl(inet_network(m_IP));
	
	if(bind(m_sock_fd, (struct sockaddr*)&m_this_addr, sizeof(m_this_addr)) == -1) {
		throw "Socket not binded";
	}
};

server::~server() {
    m_stats.clear();
    close(m_sock_fd);
}

size_t server::recv(sockaddr_in& other_addr) {
    socklen_t slen = sizeof(other_addr);
    size_t recv_len;
    
    if ((recv_len = recvfrom(m_sock_fd, m_buff, UDP_MAX_SIZE, 0,
        (struct sockaddr*)&other_addr, &slen)) == -1) {
        return -1;
    }

    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    if ((proto->type != PUT) && (proto->seq_total <= 0) && 
        (proto->seq_number > proto->seq_total)){
        return -1;
    }

    return recv_len;
};

size_t server::send(const sockaddr_in& other_addr, size_t msg_len) {
    socklen_t slen = sizeof(other_addr);

    if (sendto(m_sock_fd, m_buff, msg_len, 0, (sockaddr*)&other_addr, slen) == -1) {
        return -1;
    }

    return msg_len;
}

size_t server::construct_complete(size_t recv_len, uint32_t& crc32) {
    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    uint64_t* ID = reinterpret_cast<uint64_t*>(proto->id);
    size_t msg_len = 0;
    size_t data_len = recv_len - 17;

    crc32 = 0;
    size_t seq_pack, recv_packs, total_packs;
    uint8_t* pmem = nullptr;
    
    auto search = m_stats.find(*ID);
    if (search == m_stats.end()) {
        m_stats[*ID].m_total_packs = proto->seq_total;
    } 

    total_packs = m_stats[*ID].m_total_packs;
    recv_packs = ++m_stats[*ID].m_recv_packs;
    seq_pack = proto->seq_number;
    pmem = m_stats[*ID].m_data;

    memcpy(pmem + seq_pack * DATA_SIZE, m_buff + 17, data_len);

    proto->type = ACK;
    proto->seq_total = recv_packs;
    if (recv_packs == total_packs) {
        crc32 = crc32c(0, pmem, total_packs * DATA_SIZE);
        memcpy(m_buff + HEADER_SIZE, &crc32, sizeof(crc32));
        msg_len = HEADER_SIZE + sizeof(crc32);
    } else {
        msg_len = HEADER_SIZE;
    }

    return msg_len;
}