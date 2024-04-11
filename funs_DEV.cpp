#include "classes.hpp"
using namespace std;

void DEV::breakdown(DEV_ADDR &dev_addr, addr_t addr)
{
    dev_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    dev_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    dev_addr.index = BitSub<ADDR_SIZE, DEV_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    dev_addr.tag = BitSub<ADDR_SIZE, DEV_TAG_BITS>(addr, DEV_INDEX_BITS + WORDS_OFF + BYTES_OFF);
};

void DEV::send_rsp(uint8_t msg, uint8_t req_id, bool to_req, addr_t line_addr, line_t *line)
{
    rsp.dev_msg = msg;
    rsp.req_id = req_id;
    rsp.to_req = to_req;
    rsp.addr = line_addr;
    LineCopy(rsp.data,line);

    // while (!l2_rsp.nb_can_put()) wait();

    // l2_rsp.nb_put(rsp);
}

void DEV::snd_rsp(LLC_REQ &fwd_in)
{
    bool success = 0;
    switch (fwd_in.llc_msg)
    {
    case FWD_INV:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_INV_SPDX");
        if (reqs[reqs_fwd_stall_i].state == DEV_IS)
        {
            reqs[reqs_fwd_stall_i].state = DEV_II;
        }
        {
            // HLS_DEFINE_PROTOCOL("send rsp_inv_ack_spdx fwd_stall");
            send_rsp(RSP_INV_ACK_SPDX, 0, 0, fwd_in.addr, 0, WORD_MASK_ALL);
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
        success = true;
    }
    break;
    case FWD_REQ_O:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O");
        // Not checking word mask here
        // Fwd Req O only comes from llc, word mask should be correct
        if (reqs[reqs_fwd_stall_i].state == DEV_RI)
        {
            // HLS_DEFINE_PROTOCOL("fwd req o stall rsp o");
            // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O_RSP_O");
            send_rsp(RSP_O, fwd_in.req_id, true, fwd_in.addr, 0, fwd_in.word_mask);
            success = true;
        }
        else if (reqs[reqs_fwd_stall_i].state == DEV_XR || reqs[reqs_fwd_stall_i].state == DEV_AMO)
        {
            // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O_XR_or_AMO");
            word_mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.word_mask & (1 << i)) && state_buf[reqs[reqs_fwd_stall_i].way][i] == DEV_R)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[reqs[reqs_fwd_stall_i].way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_o on xr");
                    send_rsp(RSP_O, fwd_in.req_id, true, fwd_in.addr, line_buf[reqs[reqs_fwd_stall_i].way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_1");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
    }
    break;
    case FWD_REQ_Odata:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_Odata");
        // Not checking word mask here
        // Fwd Req Odata only comes from llc, word mask should be correct
        if (reqs[reqs_fwd_stall_i].state == DEV_RI)
        {
            // HLS_DEFINE_PROTOCOL("fwd req o stall rsp o");
            send_rsp(RSP_Odata, fwd_in.req_id, true, fwd_in.addr, reqs[reqs_fwd_stall_i].line, fwd_in.word_mask);
            success = true;
        }
        else if (reqs[reqs_fwd_stall_i].state == DEV_XR || reqs[reqs_fwd_stall_i].state == DEV_AMO)
        {
            word_mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.word_mask & (1 << i)) && state_buf[reqs[reqs_fwd_stall_i].way][i] == DEV_R)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[reqs[reqs_fwd_stall_i].way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_odata on xr");
                    send_rsp(RSP_Odata, fwd_in.req_id, true, fwd_in.addr, line_buf[reqs[reqs_fwd_stall_i].way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_2");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
    }
    break;
    case FWD_REQ_V:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_V");
        word_mask_t nack_mask = 0;
        word_mask_t ack_mask = 0;
        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            // HLS_UNROLL_LOOP(ON, "1");
            if (fwd_in.word_mask & (1 << i))
            {
                // Check if the word is in the ReqWB
                // Since Fwd Req V can be sent by other caches (NOT llc)
                // We need to handle mispredicted owner
                if ((reqs[reqs_fwd_stall_i].word_mask & (1 << i)) && reqs[reqs_fwd_stall_i].state == DEV_RI)
                {
                    ack_mask |= 1 << i;
                }
                else
                {
                    nack_mask |= 1 << i;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL();
            if (ack_mask)
            {
                send_rsp(RSP_V, fwd_in.req_id, true, fwd_in.addr, reqs[reqs_fwd_stall_i].line, ack_mask);
            }
            if (nack_mask)
            {
                // wait();
                send_rsp(RSP_NACK, fwd_in.req_id, true, fwd_in.addr, 0, nack_mask);
            }
        }
        success = true;
    }
    break;
    case FWD_RVK_O:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_RVK_O");
        if (reqs[reqs_fwd_stall_i].state == DEV_RI)
        {
            // handle DEV_RI - LLC_OV deadlock
            // HLS_DEFINE_PROTOCOL("deadlock-solver-1");
            word_mask_t rsp_mask = reqs[reqs_fwd_stall_i].word_mask & fwd_in.word_mask;
            reqs[reqs_fwd_stall_i].word_mask &= ~rsp_mask;
            if (rsp_mask)
                send_rsp(RSP_RVK_O, 0, false, addr_br.line_addr, reqs[reqs_fwd_stall_i].line, rsp_mask);
            if (!reqs[reqs_fwd_stall_i].word_mask)
                reqs[reqs_fwd_stall_i].state = DEV_II;
            success = true;
        }
        else if (reqs[reqs_fwd_stall_i].state == DEV_XR || reqs[reqs_fwd_stall_i].state == DEV_AMO)
        {
            word_mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.word_mask & (1 << i)) && state_buf[reqs[reqs_fwd_stall_i].way][i] == DEV_R)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[reqs[reqs_fwd_stall_i].way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_rvk_o_send_rsp_rvk_o");
                    send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, line_buf[reqs[reqs_fwd_stall_i].way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_3");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
    }
    break;
    case FWD_REQ_S:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_S");
        // Not checking word mask here
        // Fwd Req S only comes from llc, word mask should be correct
        if (reqs[reqs_fwd_stall_i].state == DEV_RI)
        {
            // HLS_DEFINE_PROTOCOL("fwd_req_s stall rsp_s & rsp_rvo_o");
            send_rsp(RSP_S, fwd_in.req_id, true, fwd_in.addr, reqs[reqs_fwd_stall_i].line, fwd_in.word_mask);
            // wait();
            send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, reqs[reqs_fwd_stall_i].line, fwd_in.word_mask);
            success = true;
        }
        else if (reqs[reqs_fwd_stall_i].state == DEV_XR || reqs[reqs_fwd_stall_i].state == DEV_AMO)
        {
            word_mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.word_mask & (1 << i)) && state_buf[reqs[reqs_fwd_stall_i].way][i] == DEV_R)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[reqs[reqs_fwd_stall_i].way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_s stall for xr");
                    send_rsp(RSP_S, fwd_in.req_id, true, fwd_in.addr, line_buf[reqs[reqs_fwd_stall_i].way], rsp_mask);
                    // wait();
                    send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, line_buf[reqs[reqs_fwd_stall_i].way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_4");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
    }
    break;
    case FWD_WTfwd:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_WTfwd");
        word_mask_t nack_mask = 0;

        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            // HLS_UNROLL_LOOP(ON, "1");
            if (fwd_in.word_mask & (1 << i))
            {
                if ((reqs[reqs_fwd_stall_i].word_mask & (1 << i)) && reqs[reqs_fwd_stall_i].state == DEV_RI)
                {
                    nack_mask |= 1 << i;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL();
            if (nack_mask)
            {
                // wait();
                send_rsp(RSP_NACK, fwd_in.req_id, true, fwd_in.addr, 0, nack_mask);
            }
        }

        success = true;
    }

    break;
    default:
        break;
    }
};