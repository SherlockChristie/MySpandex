#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void DEV::dev_caller_tu() {}
void DEV::breakdown(DEV_ADDR &dev_addr, addr_t addr)
{
    dev_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    dev_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    dev_addr.index = BitSub<ADDR_SIZE, DEV_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    dev_addr.tag = BitSub<ADDR_SIZE, DEV_TAG_BITS>(addr, DEV_INDEX_BITS + WORDS_OFF + BYTES_OFF);
}

bool DEV::fetch_line(DEV_ADDR &dev_addr, DATA_LINE &dev_line)
{
    line_t zero = {0};
    unsigned long dev_index = (dev_addr.index).to_ulong();
    dev_line.line_state = state_buf[dev_index];
    dev_line.sharers = sharers_buf[dev_index];
    if ((tag_buf[dev_index] != dev_addr.tag) || (dev_line.line_state == DEV_I))
    {
        LineCopy(dev_line.data, zero);
        return 0;
    }
    else
    {
        LineCopy(dev_line.data, cache[dev_index]);
        return 1;
    }
}

// void DEV::RSP_msg(TU &REQor)
// {
//     gen.msg = msg;
//     rsp.REQ_id = REQ_id;
//     rsp.to_REQ = to_REQ;
//     rsp.addr = line_addr;
//     LineCopy(gen.data, line);

//     // while (!l2_rsp.nb_can_put()) wait();

//     // l2_rsp.nb_put(RSP);
// }

// void DEV::rcv_fwd(MSG &fwd_in)
// {
//     unsigned long id = dev_id.to_ulong();
//     req_buf.push_back(fwd_in);
//     breakdown(dev_addr, fwd_in.addr);
//     fetch_line(dev_addr, dev_line);
//     tus[id].mapping_wrapper(devs[id]);
// }
