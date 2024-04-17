#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void DEV::dev_caller_tu(){}
void DEV::solve_pending_REQWB(){}
void DEV::breakdown(DEV_ADDR &dev_addr, addr_t addr)
{
    dev_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    dev_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    dev_addr.index = BitSub<ADDR_SIZE, DEV_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    dev_addr.tag = BitSub<ADDR_SIZE, DEV_TAG_BITS>(addr, DEV_INDEX_BITS + WORDS_OFF + BYTES_OFF);
}

bool DEV::fetch_line(DEV_ADDR &dev_addr, DATA_LINE &dev_data)
{
    line_t zero = {0};
    unsigned long dev_index = (dev_addr.index).to_ulong();
    dev_data.state = state_buf[dev_index];
    dev_data.sharers = sharers_buf[dev_index];
    if ((tag_buf[dev_index] != dev_addr.tag) || (dev_data.state == DEV_I))
    {
        LineCopy(dev_data.data, zero);
        return 0;
    }
    else
    {
        LineCopy(dev_data.data, cache[dev_index]);
        return 1;
    }
}

// void DEV::RSP_msg(TU &REQor)
// {
//     rsp.msg = msg;
//     rsp.REQ_id = REQ_id;
//     rsp.to_REQ = to_REQ;
//     rsp.addr = line_addr;
//     LineCopy(rsp.data, line);

//     // while (!l2_rsp.nb_can_put()) wait();

//     // l2_rsp.nb_put(RSP);
// }

void DEV::rcv_fwd(MSG &fwd_in)
{
    TU reqor;
    breakdown(dev_addr, fwd_in.addr);
    fetch_line(dev_addr, dev_data);

    switch (fwd_in.msg)
    {
    case FWD_REQ_V:
        // Forwards to owner; Others can read; Remain O state;
        {
            // Just right in the expected state;
            if (dev_data.state == DEV_O)
            {
                rsp.msg = RSP_V;
                LineCopy(rsp.data, dev_data.data_line);
            }
            // Pending Transition to Expected State;
            else if (dev_data.state == DEV_XO)
            {
                wait();
            }
            // Pending Transition from Expected State;
            else if (dev_data.state == DEV_OI)
            {
                solve_pending_REQWB();
            }
            else
            {
                // no ,tu do this, go to tu;
                // do nack;
                // if > MAX_RETRY REQV = REQWT;
                dev_caller_tu();
            }
            rsp.to_reqor = 1;
            dev_data.state == DEV_O;
            break;
        }
    case FWD_REQ_O:
    {
        // Just right in the expected state;
        if (dev_data.state == DEV_O || dev_data.state == DEV_XO)
        {
            rsp.msg = RSP_O;
            // no need for pending states, since REQO does not need a data transfer, just ownership;
        }
        // Pending Transition from Expected State;
        else if (dev_data.state == DEV_OI)
        {
            solve_pending_REQWB();
        }
        rsp.to_reqor = 1;
        dev_data.state == DEV_I;
        break;
    }
    case FWD_REQ_Odata:
    {
        if (dev_data.state == DEV_O)
        {
            rsp.msg = RSP_Odata;
            LineCopy(rsp.data, dev_data.data_line);
        }
        else if (dev_data.state == DEV_XO)
        // that's how FWD_REQ_Odata diff from FWD_REQ_O;
        {
            wait();
        }
        else if (dev_data.state == DEV_OI)
        {
            solve_pending_REQWB();
        }
        rsp.to_reqor = 1;
        dev_data.state == DEV_I;
        break;
    }
    case FWD_RVK_O:
    {
        if (dev_data.state == DEV_O)
        {
            rsp.msg = RSP_RVK_O;
        }
        else if (dev_data.state == DEV_XO)
        {
            wait();
        }
        else if (dev_data.state == DEV_OI)
        {
            solve_pending_REQWB();
        }
        rsp.to_reqor = 0;
        dev_data.state == DEV_I;
        break;
    }
    case FWD_INV:
    {
        if (dev_data.state == DEV_S || dev_data.state == DEV_IS || dev_data.state == DEV_XO)
        {
            rsp.msg = RSP_INV_ACK;
        }
        rsp.to_reqor = 0;
        dev_data.state == DEV_I;
        break;
    }

    case FWD_REQ_S:
    {
        if (dev_data.state == DEV_O)
        {
            rsp.to_reqor = 1;
            rsp.msg = RSP_S;
            // another response ???
        }
        else if (dev_data.state == DEV_XO)
        {
            wait();
        }
        else if (dev_data.state == DEV_OI)
        {
            solve_pending_REQWB();
        }
        break;
    }
    // case FWD_WTfwd:
    // {
    //     break;
    // }
    default:
        break;
    }
}
