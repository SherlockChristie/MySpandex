#include <cstdint> // For uint8_t, uint32_t etc.
#include <cmath>
#include <bitset>
#include "headers.hpp"
using namespace std;

/*
 * Cache data types
 */
typedef uint8_t byte_t;
typedef byte_t word_t[BYTES_PER_WORD];
typedef word_t line_t[WORDS_PER_LINE];

typedef bitset<ADDR_SIZE> addr_t;

typedef bitset<STATE_BITS> state_t;
typedef bitset<DEV_TAG_BITS> dev_tag_t;
typedef bitset<LLC_TAG_BITS> llc_tag_t;
typedef bitset<DEV_INDEX_BITS> dev_index_t;
typedef bitset<LLC_INDEX_BITS> llc_index_t;
typedef bitset<MAX_DEVS> sharers_t;

typedef bitset<WORDS_OFF> word_offset;
typedef bitset<BYTES_OFF> byte_offset;
typedef bitset<10> req_type;

struct DEV_ADDR
{
    dev_tag_t tag;
    dev_index_t index;
    word_offset w_off;
    byte_offset b_off;
};

struct DEV_REQ
{
    addr_t addr;
    uint8_t dev_msg; // Request type: read, write or RMW;
};
struct DEV_RSP
{

};

struct TU_REQ
{
    addr_t addr;
    uint8_t tu_msg; // Translate device message into LLC message.(Table II)
    bool gran;      // 0 for word granularity, 1 for line granularity;
};
struct TU_RSP
{

};

struct LLC_ADDR
{
    llc_tag_t tag;
    llc_index_t index;
    word_offset w_off;
    byte_offset b_off;
};

struct LLC_REQ
{
    addr_t addr;

};
struct LLC_RSP
{
    

};

struct LLC_DATA
{
    bool hit;
    word_t data_line[WORDS_PER_LINE]; // word granularity;
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