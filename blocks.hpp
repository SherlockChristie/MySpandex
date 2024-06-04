#ifndef __BLOCKS_HPP__
#define __BLOCKS_HPP__

#include "headers.hpp"
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

// typedef bitset<STATE_DEV> dev_state_t;
typedef bitset<STATE_LINE> state_t;
typedef bitset<STATE_WORDS> spx_word_state_t;

// the size of word_state_t should be max(STATE_LINE,STATE_DEV);
// typedef bitset<STATE_DEV> word_state_t;
typedef bitset<STATE_UNSTABLE> unstable_state_t;
typedef bitset<DEV_TAG_BITS> dev_tag_t;
typedef bitset<LLC_TAG_BITS> llc_tag_t;
typedef bitset<DEV_INDEX_BITS> dev_index_t;
typedef bitset<LLC_INDEX_BITS> llc_index_t;
// One-hot for sharers; e.g. 0b0110 indicates that dev_2 and dev_1 shares it;
typedef bitset<MAX_DEVS> id_bit_t;
// typedef bitset<MAX_DEVS_BITS> id_bit_t;
// This is a snooping-based protocol, you just broadcast the message;
typedef bitset<MAX_DEVS_BITS> id_num_t;

typedef bitset<WORDS_PER_LINE> mask_t;
typedef bitset<WORDS_OFF> word_offset_t;
typedef bitset<BYTES_OFF> byte_offset_t;
// typedef bitset<10> req_type;

string msg_which(int msg);
string dev_which(int id);

// Does not need DEV_DATA, TU_DATA, LLC_DATA; just DATA_LINE or DATA_WORD;
// Data is always the same; it depends on how we explain the address type.
struct DATA_LINE
{
    // bool hit;
    // word_t data_line[WORDS_PER_LINE]; // word granularity;
    line_t data;
    state_t line_state;
    // GPU and MESI only use line_state;
    // DeNovo. LLC and TU uses whole state_t(line_state & word_state);
    spx_word_state_t word_state;
    id_bit_t sharers;

    void line_display()
    {
        std::cout << "  DATA_LINE_DISPLAY----------------------------------" << std::endl;
        std::cout << "  data: ";
        for (int i = BYTES_PER_WORD * WORDS_PER_LINE - 1; i >= 0; i--)
        {
            printf("%x ", data[i]);
        }
        std::cout << std::endl;
        std::cout << "  line_state: " << line_state << std::endl;
        std::cout << "  word_state: " << word_state << std::endl;
        std::cout << "  sharers: " << sharers << std::endl;
        std::cout << "  ---------------------------------------------------" << std::endl;
    }
};

struct DATA_WORD
{
    // addr_t addr;
    word_t data;
    state_t state;
    // id_bit_t sharers;
    // No, Spandex stores sharers for the whole line;
    // Also do not need owners_t, since if in O, data field itself stores the owner id;
    // MESI won't use DATA_WORD;
    // DeNovo do the same with Spandex;
    // GPU coh. does not have state S or O;

    void word_clear()
    {
        for (int i = 0; i < BYTES_PER_WORD; i++)
        {
            data[i] = 0;
        }
        state = SPX_I;
    }
    void word_display()
    {
        std::cout << "  DATA_WORD_DISPLAY----------------------------------" << std::endl;
        std::cout << "  data: ";
        for (int i = BYTES_PER_WORD - 1; i >= 0; i--)
        {
            printf("%x ", data[i]);
        }
        std::cout << std::endl;
        std::cout << "  state: " << state << std::endl;
        std::cout << "  ---------------------------------------------------" << std::endl;
    }
};

struct MSG
{
    int id; // Every req/prb has a unique id, rsp has an id same with its corrsponding rsp/prb.
    // LSB 00:line/word0?
    mask_t mask;   // bitmask;
    id_num_t src;  // self number;
    id_bit_t dest; // Destination"s" of the request;
    addr_t addr;   // Used when it needs data instead of just ownership.
    int msg;
    // dev_req: Read, write or RMW;
    // tu_req: Translate device message into LLC message.(Table II)
    // llc_req: Fordward message;
    bool gran; // 0 for word granularity, 1 for line granularity;
    // word_offset_t offset; // Used when it is a line granularity req.
    // no need, offset only for one, but mask_t for multiple words in a line;
    mask_t ok_mask; // if all the line is ready;
    unstable_state_t u_state;
    // Store the transient states in the req that triggers it instead of the LLC self.
    DATA_LINE data_line;
    DATA_WORD data_word;
    int retry_times = 0;
    int time_hm = 0; // The simulator to record the latency for H-MESI.
    int time_sp = 0; // The simulator to record the latency for Spandex.

    void msg_display()
    {
        std::cout << "MSG_DISPLAY------------------------------------------" << endl;
        std::cout << "id: " << id << std::endl;
        std::cout << "mask: " << mask << std::endl;
        std::cout << "src: " << dev_which(src.to_ulong()) << std::endl;
        std::cout << "dest: " << dest << std::endl;
        std::cout << "addr: " << addr << std::endl;
        std::cout << "msg: " << msg_which(msg) << std::endl;
        if (gran)
            std::cout << "gran: line" << std::endl;
        else
            std::cout << "gran: word" << std::endl;
        std::cout << "ok_mask: " << ok_mask << std::endl;
        std::cout << "u_state: " << u_state << std::endl;
        data_line.line_display();
        data_word.word_display();
        std::cout << "retry_times: " << retry_times << std::endl;
        std::cout << "H-MESI: " << time_hm << std::endl;
        std::cout << "Spandex: " << time_sp << std::endl;
        std::cout << "-----------------------------------------------------" << endl;
    }
};

// Does not need DEV_REQ, TU_REQ, LLC_REQ;
// struct REQ
// {
//     id_bit_t dest;   // Destination of the request;
//     addr_t addr; // Used when it needs data instead of just ownership.
//     int msg;
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
//     // int req_id;
//     id_bit_t dest;
//     addr_t addr;
//     // bool to_reqor;
//     int msg; // Reponse type;
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

    void addr_display()
    {
        std::cout << "  DEV_ADDR_DISPLAY-----------------------------------" << std::endl;
        std::cout << "  tag: " << tag << std::endl;
        std::cout << "  index: " << index << std::endl;
        std::cout << "  word_offset: " << w_off << std::endl;
        std::cout << "  byte_offset_t: " << b_off << std::endl;
        std::cout << "  ---------------------------------------------------" << std::endl;
    }
};

struct LLC_ADDR
{
    llc_tag_t tag;
    llc_index_t index;
    word_offset_t w_off;
    byte_offset_t b_off;

    void addr_display()
    {
        std::cout << "  LLC_ADDR_DISPLAY-----------------------------------" << std::endl;
        std::cout << "  tag: " << tag << std::endl;
        std::cout << "  index: " << index << std::endl;
        std::cout << "  word_offset: " << w_off << std::endl;
        std::cout << "  byte_offset_t: " << b_off << std::endl;
        std::cout << "  ---------------------------------------------------" << std::endl;
    }
};

#endif // __BLOCKS_HPP__