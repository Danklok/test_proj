#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "client.hpp"
#include "../crc32.hpp"

const size_t TIMEOUT_SECONDS = 1;
const uint16_t PORT = 6543;
const char     IP[] = "127.0.0.1";

File::File(const std::string& filename) {
    if((m_file_fd = open(filename.c_str(), O_RDONLY)) == -1)
        throw "Can not open file";
    struct stat stat_buf;
    fstat(m_file_fd, &stat_buf);
    m_file_size = stat_buf.st_size;
    m_last_pack_size = m_file_size % DATA_SIZE;
    m_total_packs = m_file_size / DATA_SIZE;
    if (m_last_pack_size != 0) {
        ++m_total_packs;
    } 

    std::vector<size_t> temp(m_total_packs);
    std::iota(temp.begin(), temp.end(), 0);
    std::shuffle(temp.begin(), temp.end(), std::mt19937{std::random_device{}()});
    m_seq_numbers = std::move(temp);

    memcpy(m_ID, filename.substr(filename.size() - ID_SIZE).c_str(), ID_SIZE);
    
    m_filebuff = static_cast<uint8_t*>(mmap(nullptr, m_file_size, 
                                        PROT_READ, MAP_FILE | MAP_SHARED, m_file_fd, 0));         
    m_crc32 = crc32c(0, m_filebuff, m_file_size);
} 

File::~File() {
    if (m_file_fd != -1) {
        munmap(m_filebuff, m_file_size);
        close(m_file_fd);
    }  
} 

Client::Client(const std::string& filename) : m_IP(IP), m_PORT(PORT) {
	if ((m_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		throw "Socket not created";
	}

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    setsockopt(m_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	
	memset((char*)&m_server_addr, 0, sizeof(m_server_addr));
	
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(m_PORT);
	m_server_addr.sin_addr.s_addr = htonl(inet_network(m_IP));

    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } 

    std::string filepath(std::string(cwd) + '/' + filename);
    files[0] = std::make_shared<File>(filepath);
};

size_t Client::recv() {
    socklen_t slen = sizeof(m_server_addr);
    size_t recv_len;
    
    if ((recv_len = recvfrom(m_sock_fd, m_buff, UDP_MAX_SIZE, 0,
        (struct sockaddr*)&m_server_addr, &slen)) == -1) {
        m_ticket_recied = false;
        return -1;
    }

    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    auto file = files[0];
    auto ID_compare = [&]()->bool {
        for (size_t i = 0; i < ID_SIZE; ++i) {
            if (proto->id[i] != file->m_ID[i]) {
                return false;
            }
        }
        return true;
    };

    if ((proto->type != ACK) || (proto->seq_number != 
        file->m_seq_numbers.back()) || ID_compare()) {
        m_ticket_recied = false;
        return -1;
    }

    return recv_len;
};

size_t Client::send() {
    Proto* proto = reinterpret_cast<Proto*>(m_buff);
    auto file = files[0];
    socklen_t slen = sizeof(m_server_addr);
    size_t msg_len, send_len;
  
    if (!file->m_seq_numbers.empty()) {
        memcpy(proto->id, file->m_ID, ID_SIZE);
        proto->type = PUT;
        proto->seq_total = file->m_total_packs;
        if (m_ticket_recied) {
            file->m_seq_numbers.pop_back();
        }
        proto->seq_number = file->m_seq_numbers.back();   
        
        if (proto->seq_number == (proto->seq_total - 1)) {
            msg_len = file->m_last_pack_size;
        } else {
            msg_len = DATA_SIZE;
        }

        u_int8_t* seg_addr = file->m_filebuff + proto->seq_number * DATA_SIZE;
        memcpy(m_buff + HEADER_SIZE, seg_addr, msg_len);

        if (send_len = sendto(m_sock_fd, m_buff, msg_len, 0, 
                  (sockaddr*)&m_server_addr, slen) == -1) {
            return -1;
        }
    } else {
        return -1;
    }

    return send_len;
}

Client::~Client() {
    if (m_sock_fd != -1) {
        close(m_sock_fd);
    }
}
