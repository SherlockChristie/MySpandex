void DEV::snd_rsp_learn(LLC_REQ &fwd_in)
// Behaviour when DEV receives an external request from TU (Table IV).
{
    state_t state;
    bool success = 0;
    switch (fwd_in.llc_msg)
    {
    case FWD_REQ_V:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_V");
        word_offset_t nack_mask = 0;
        word_offset_t ack_mask = 0;
        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            word_offset_t i_to_bin(i);
            // HLS_UNROLL_LOOP(ON, "1");
            if (fwd_in.mask == i_to_bin)
            {
                // Check if the word is in the ReqWB
                // Since Fwd Req V can be sent by other caches (NOT llc)
                // We need to handle mispredicted owner
                if ((req.mask == i_to_bin) && (state == DEV_OI))
                {
                    ack_mask == i_to_bin;
                }
                else
                {
                    nack_mask == i_to_bin;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL();
            if (ack_mask)
            {
                send_rsp(RSP_V, fwd_in.req_id, true, fwd_in.addr, req.line, ack_mask);
            }
            if (nack_mask)
            {
                // wait();
                send_rsp(RSP_NACK, fwd_in.req_id, true, fwd_in.addr, 0, nack_mask);
            }
        }
        success = 1;
        break;
    }
    case FWD_REQ_O:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O");
        // Not checking word mask here
        // Fwd Req O only comes from llc, word mask should be correct
        // Pending Transition from Expected State;
        if (state == DEV_OI)
        {
            // HLS_DEFINE_PROTOCOL("fwd req o stall rsp o");
            // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O_RSP_O");
            send_rsp(RSP_O, fwd_in.req_id, true, fwd_in.addr, 0, fwd_in.mask);
            success = true;
        }
        // Pending Transition to Expected State;;
        else if (state == DEV_XO || state == DEV_AMO)
        {
            // ADD_COVERAGE("do_fwd_fwd_stall_REQ_O_XR_or_AMO");
            mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.mask & (1 << i)) && state_buf[req.way][i] == DEV_O)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[req.way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_o on xr");
                    send_rsp(RSP_O, fwd_in.req_id, true, fwd_in.addr, line_buf[req.way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_1");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }break;
    }
    
    case FWD_REQ_Odata:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_Odata");
        // Not checking word mask here
        // Fwd Req Odata only comes from llc, word mask should be correct
        // Pending Transition from Expected State;
        if (state == DEV_OI)
        {
            // HLS_DEFINE_PROTOCOL("fwd req o stall rsp o");
            send_rsp(RSP_Odata, fwd_in.req_id, true, fwd_in.addr, req.line, fwd_in.mask);
            success = true;
        }
        // Pending Transition to Expected State;
        else if (state == DEV_XO || state == DEV_AMO)
        {
            mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.mask & (1 << i)) && state_buf[req.way][i] == DEV_O)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[req.way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_odata on xr");
                    send_rsp(RSP_Odata, fwd_in.req_id, true, fwd_in.addr, line_buf[req.way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_2");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
        break;
    }

    case FWD_RVK_O:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_RVK_O");
        if (state == DEV_OI)
        {
            // handle DEV_OI - LLC_OV deadlock
            // HLS_DEFINE_PROTOCOL("deadlock-solver-1");
            mask_t rsp_mask = req.mask & fwd_in.mask;
            req.mask &= ~rsp_mask;
            if (rsp_mask)
                send_rsp(RSP_RVK_O, 0, false, addr_br.line_addr, req.line, rsp_mask);
            if (!req.mask)
                state = DEV_II;
            success = true;
        }
        else if (state == DEV_XR || state == DEV_AMO)
        {
            mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.mask & (1 << i)) && state_buf[req.way][i] == DEV_O)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[req.way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_rvk_o_send_rsp_rvk_o");
                    send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, line_buf[req.way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_3");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
        break;
    }

    case FWD_INV:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_INV_SPDX");
        if (state == DEV_IS)
        {
            state = DEV_II;
        }
        {
            // HLS_DEFINE_PROTOCOL("send rsp_inv_ack_spdx fwd_stall");
            send_rsp(RSP_INV_ACK_SPDX, 0, 0, fwd_in.addr, 0, mask_ALL);
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
        success = 1;
        break;
    }

    case FWD_REQ_S:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_REQ_S");
        // Not checking word mask here
        // Fwd Req S only comes from llc, word mask should be correct
        if (state == DEV_OI)
        {
            // HLS_DEFINE_PROTOCOL("fwd_req_s stall rsp_s & rsp_rvo_o");
            send_rsp(RSP_S, fwd_in.req_id, true, fwd_in.addr, req.line, fwd_in.mask);
            // wait();
            send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, req.line, fwd_in.mask);
            success = true;
        }
        else if (state == DEV_XR || state == DEV_AMO)
        {
            mask_t rsp_mask = 0;
            if (tag_hit)
            {
                for (int i = 0; i < WORDS_PER_LINE; i++)
                {
                    // HLS_UNROLL_LOOP(ON, "1");
                    if ((fwd_in.mask & (1 << i)) && state_buf[req.way][i] == DEV_O)
                    {
                        rsp_mask |= 1 << i;
                        state_buf[req.way][i] = DEV_I;
                    }
                }
                if (rsp_mask)
                {
                    // HLS_DEFINE_PROTOCOL("fwd_req_s stall for xr");
                    send_rsp(RSP_S, fwd_in.req_id, true, fwd_in.addr, line_buf[req.way], rsp_mask);
                    // wait();
                    send_rsp(RSP_RVK_O, fwd_in.req_id, false, fwd_in.addr, line_buf[req.way], rsp_mask);
                    success = true;
                }
            }
        }
        {
            // HLS_DEFINE_PROTOCOL("send_invalidate_4");
            // wait();
            send_inval(fwd_in.addr, DATA);
        }
        break;
    }

    case FWD_WTfwd:
    {
        // ADD_COVERAGE("do_fwd_fwd_stall_WTfwd");
        mask_t nack_mask = 0;

        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            // HLS_UNROLL_LOOP(ON, "1");
            if (fwd_in.mask & (1 << i))
            {
                if ((req.mask & (1 << i)) && state == DEV_OI)
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

        success = 1;
        break;
    }

    default:
        break;
    }
};
