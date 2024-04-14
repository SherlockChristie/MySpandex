#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void DEV::breakdown(DEV_ADDR &dev_addr, addr_t addr)
{
    dev_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    dev_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    dev_addr.index = BitSub<ADDR_SIZE, DEV_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    dev_addr.tag = BitSub<ADDR_SIZE, DEV_TAG_BITS>(addr, DEV_INDEX_BITS + WORDS_OFF + BYTES_OFF);
};

void DEV::send_rsp(uint8_t msg, uint8_t req_id, bool to_req, addr_t line_addr, line_t &line)
{
    rsp.dev_msg = msg;
    rsp.req_id = req_id;
    rsp.to_req = to_req;
    rsp.addr = line_addr;
    LineCopy(rsp.data, line);

    // while (!l2_rsp.nb_can_put()) wait();

    // l2_rsp.nb_put(rsp);
}

void DEV::snd_rsp(LLC_REQ &fwd_in)
{}
