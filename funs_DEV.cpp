#include "classes.hpp"
using namespace std;
void DEV_ADDR::breakdown(addr_t addr)
{
    b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    index = BitSub<ADDR_SIZE, DEV_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    tag = BitSub<ADDR_SIZE, DEV_TAG_BITS>(addr, DEV_INDEX_BITS + WORDS_OFF + BYTES_OFF);
};