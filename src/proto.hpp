#ifndef _PROTO_H_
#define _PROTO_H_

#include <cstddef>
#include <cstdint>

const uint8_t ACK = 0;
const uint8_t PUT = 1; 
const size_t UDP_MAX_SIZE = 1472;
const size_t HEADER_SIZE = 17;
const size_t DATA_SIZE = 1472 - HEADER_SIZE;
const size_t ID_SIZE = 8;

struct __attribute__((__packed__)) Proto {
    uint32_t seq_number;        // номер пакета
    uint32_t seq_total;         // количество пакетов с данными
    uint8_t  type;              // тип пакета: 0 == ACK, 1 == PUT
    uint8_t  id[ID_SIZE];       // 8 байт - идентификатор, отличающий один файл от другого
    uint8_t  data[DATA_SIZE];   // после заголовка и до конца UDP пакета идут данные
};

#endif