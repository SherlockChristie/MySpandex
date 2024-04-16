#ifndef __BLOCKS_HPP__
#define __BLOCKS_HPP__

#include <cstdint> // For uint8_t, uint32_t etc.
// #include <cmath>
#include <bitset>
#include "consts.hpp"
using namespace std;

/*
 * Cache data types
 */
typedef uint8_t byte_t;
// typedef byte_t word_t[BYTES_PER_WORD];
// typedef word_t line_t[WORDS_PER_LINE];
// 不要用二级数组来定义，会引发令人恼怒的指针问题;
typedef byte_t word_t[BYTES_PER_WORD];
typedef byte_t line_t[BYTES_PER_WORD * WORDS_PER_LINE];

// bitset头文件只支持相同位宽的操作数操作，不能像Verilog自动补0，写bitset函数十分不方便，还是用数组吧;
// 干脆不用数组的思想而用位掩码的思想;
// typedef bitset<BITS_PER_BYTE> byte_t;
// typedef bitset<BITS_PER_BYTE*BYTES_PER_WORD> word_t;
// typedef bitset<BITS_PER_BYTE*BYTES_PER_WORD*WORDS_PER_LINE> line_t;

typedef bitset<ADDR_SIZE> addr_t;

typedef bitset<STATE_BITS> state_t;
typedef bitset<DEV_TAG_BITS> dev_tag_t;
typedef bitset<LLC_TAG_BITS> llc_tag_t;
typedef bitset<DEV_INDEX_BITS> dev_index_t;
typedef bitset<LLC_INDEX_BITS> llc_index_t;
typedef bitset<MAX_DEVS> sharers_t;
// One-hot for sharers; e.g. 0b110 indicates that dev_2 and dev_1 shares it;
typedef bitset<MAX_DEVS_BITS> id_t;

typedef bitset<WORDS_OFF> word_offset_t;
typedef bitset<BYTES_OFF> byte_offset_t;
typedef bitset<10> REQ_type;

struct DEV_ADDR
{
    dev_tag_t tag;
    dev_index_t index;
    word_offset_t w_off;
    byte_offset_t b_off;
};

struct DEV_DATA
{
    // bool hit;
    // word_t data_line[WORDS_PER_LINE]; // word granularity;
    line_t data_line;
    state_t state;
    sharers_t sharers;
};

struct DEV_REQ
{
    id_t dest; // Destination of the REQuest;
    addr_t addr;
    uint8_t dev_msg; // REQuest type: read, write or RMW;
    bool gran;       // 0 for word granularity, 1 for line granularity;
    word_offset_t mask;
};
struct DEV_RSP
{
    // uint8_t REQ_id;
    // addr_t addr;
    bool to_reqor;
    uint8_t dev_msg; // Reponse type;
    line_t data;
};

struct TU_REQ
{
    id_t dest;
    addr_t addr;
    uint8_t tu_msg; // Translate device message into LLC message.(Table II)
    bool gran;      // 0 for word granularity, 1 for line granularity;
    word_offset_t mask;
};

struct TU_RSP
{
    uint8_t tu_msg;
    line_t data;
};

struct TU_DATA
{
    line_t data_line;
    state_t state;
    sharers_t sharers;
};

struct LLC_ADDR
{
    llc_tag_t tag;
    llc_index_t index;
    word_offset_t w_off;
    byte_offset_t b_off;
};

struct LLC_REQ
{
    uint8_t llc_msg; // Translate device message into LLC message.(Table II)
    bool gran;       // 0 for word granularity, 1 for line granularity;
    id_t dest; // Destination of the request;
    word_offset_t mask;
    addr_t addr; // Used when it needs data instead of just ownership.
};
struct LLC_RSP
{
    uint8_t llc_msg;
    line_t data;
};

struct LLC_DATA
{
    // bool hit;
    // word_t data_line[WORDS_PER_LINE]; // word granularity;
    line_t data_line;
    state_t state;
    sharers_t sharers;
};

struct DATA_WORD
{
    // addr_t addr;
    state_t state;
    word_t data;
};

struct DATA_LINE
{
    // addr_t addr;
    state_t state;
    line_t data;
};

#endif // __BLOCKS_HPP__