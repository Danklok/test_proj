#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "server.hpp"
#include "../crc32.hpp"

server::server() : m_IP(IP), m_PORT(PORT) {
	if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		throw "Socket not created";
	}
	
	memset((char*)&m_client_addr, 0, sizeof(m_client_addr));
	
	m_client_addr.sin_family = AF_INET;
	m_client_addr.sin_port = htons(m_PORT);
	m_client_addr.sin_addr.s_addr = htonl(inet_network(m_IP));
	
	if(bind(m_sock_fd, (struct sockaddr*)&m_client_addr, sizeof(m_client_addr)) == -1) {
		throw "Socket not binded";
	}
};

server::~server() {
    m_stats.clear();
    if (m_sock_fd != -1) {
        close(m_sock_fd);
    }
}

size_t server::recv() {
    socklen_t slen = sizeof(m_client_addr);
    size_t recv_len;
    
    if ((recv_len = recvfrom(m_sock_fd, m_buff, UDP_MAX_SIZE, 0,
        (struct sockaddr*)&m_client_addr, &slen)) == -1) {
        return -1;
    }

    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    if ((proto->type != PUT) || (proto->seq_total <= 0) ||
        (proto->seq_number > proto->seq_total)){
        return -1;
    }

    return recv_len;
};

size_t server::send(size_t msg_len) {
    size_t recv_len = 0;
    socklen_t slen = sizeof(m_client_addr);
    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    proto->type = ACK;

    if (sendto(m_sock_fd, m_buff, msg_len, 0, (sockaddr*)&m_client_addr, slen) == -1) {
        return -1;
    }
    // proto->seq_total = recv_packs;

    return msg_len;
}

size_t server::construct_complete(size_t recv_size, uint32_t& crc32) {
    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    uint64_t* pID = reinterpret_cast<uint64_t*>(proto->id);
    uint64_t ID = *pID;  
    size_t data_size = recv_size - HEADER_SIZE;
    size_t seq_number, recv_packs, recv_bytes, total_packs;
    uint8_t* p_mem = nullptr;
    size_t msg_size = 0;

    total_packs = proto->seq_total;
    
    auto stat = m_stats.find(ID);
    if (stat == m_stats.end()) {
        std::shared_ptr<Mempool> mempool = std::make_shared<Mempool>(total_packs);
        m_stats.emplace(std::pair<uint64_t, std::shared_ptr<Mempool>>{ID, mempool});                          
    }
    auto& mempool = *m_stats[ID];
   
    seq_number = proto->seq_number;
    p_mem = mempool.m_data;
    
    auto file_part = mempool.m_recv_file_parts.emplace(seq_number);
    if (std::get<1>(file_part)) {
        recv_packs = ++mempool.m_recv_packs;
        mempool.m_recv_bytes += data_size;
        memcpy(p_mem + seq_number * DATA_SIZE, m_buff + HEADER_SIZE, data_size);
    }

    if(recv_packs == total_packs) {
        crc32 = crc32c(0, p_mem,  mempool.m_recv_bytes);
        memcpy(m_buff + HEADER_SIZE, &crc32, sizeof(crc32));
        msg_size = HEADER_SIZE + sizeof(crc32);
    } else {
        msg_size = HEADER_SIZE;
    }

    return msg_size;
}

bool server::check_hash() {

}