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

typedef bitset<STATE_DEV> dev_state_t;
typedef bitset<STATE_LINE> spx_line_state_t;
// the size of word_state_t should be max(STATE_LINE,STATE_DEV);
typedef bitset<STATE_WORDS> spx_word_state_t;

typedef bitset<STATE_DEV> word_state_t;
typedef bitset<STATE_UNSTABLE> unstable_state_t;
typedef bitset<DEV_TAG_BITS> dev_tag_t;
typedef bitset<LLC_TAG_BITS> llc_tag_t;
typedef bitset<DEV_INDEX_BITS> dev_index_t;
typedef bitset<LLC_INDEX_BITS> llc_index_t;
// One-hot for sharers; e.g. 0b0110 indicates that dev_2 and dev_1 shares it;
typedef bitset<MAX_DEVS> sharers_t;
// typedef bitset<MAX_DEVS_BITS> id_t;
// This is a snooping-based protocol, you just broadcast the message;
typedef sharers_t id_t;

typedef bitset<WORDS_OFF> word_offset_t;
typedef bitset<BYTES_OFF> byte_offset_t;
// typedef bitset<10> req_type;

// Does not need DEV_DATA, TU_DATA, LLC_DATA; just DATA_LINE or DATA_WORD;
// Data is always the same; it depends on how we explain the address type.
struct DATA_LINE
{
    // bool hit;
    // word_t data_line[WORDS_PER_LINE]; // word granularity;
    line_t data;
    spx_line_state_t line_state;
    // GPU and MESI only use line_state;
    // DeNovo. LLC and TU uses whole state_t(line_state & word_state);
    spx_word_state_t word_state;
    sharers_t sharers;
};

struct DATA_WORD
{
    // addr_t addr;
    word_t data;
    word_state_t state;
    // sharers_t sharers;
    // No, Spandex stores sharers for the whole line;
    // Also do not need owners_t, since if in O, data field itself stores the owner id;
    // MESI won't use DATA_WORD;
    // DeNovo do the same with Spandex;
    // GPU coh. does not have state S or O;
};

struct MSG
{
    id_t dest;   // Destination"s" of the request;
    addr_t addr; // Used when it needs data instead of just ownership.
    uint8_t msg;
    // dev_req: Read, write or RMW;
    // tu_req: Translate device message into LLC message.(Table II)
    // llc_req: Fordward message;
    bool gran;          // 0 for word granularity, 1 for line granularity;
    word_offset_t mask; // Used when it is a line granularity req.
    unstable_state_t u_state;
    // Store the transient states in the req that triggers it instead of the LLC self.
    DATA_LINE data_line;
    DATA_WORD data_word;
};

// Does not need DEV_REQ, TU_REQ, LLC_REQ;
// struct REQ
// {
//     id_t dest;   // Destination of the request;
//     addr_t addr; // Used when it needs data instead of just ownership.
//     uint8_t msg;
//     // dev_msg: Read, write or RMW;
//     // tu_msg: Translate device message into LLC message.(Table II)
//     // llc_msg: Fordward message;
//     bool gran;          // 0 for word granularity, 1 for line granularity;
//     word_offset_t mask; // Used when it is a line granularity req.
//     unstable_state_t u_state;
//     // Store the transient states in the req that triggers it instead of the LLC self.
// };
// struct RSP
// {
//     // uint8_t req_id;
//     id_t dest;
//     addr_t addr;
//     // bool to_reqor;
//     uint8_t msg; // Reponse type;
//     bool gran;
//     word_offset_t mask;
//     DATA_LINE data_line;
//     DATA_WORD data_word;
// };

struct DEV_ADDR
{
    dev_tag_t tag;
    dev_index_t index;
    word_offset_t w_off;
    byte_offset_t b_off;
};

struct LLC_ADDR
{
    llc_tag_t tag;
    llc_index_t index;
    word_offset_t w_off;
    byte_offset_t b_off;
};

#endif // __BLOCKS_HPP__