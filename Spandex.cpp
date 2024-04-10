#include "classes.hpp"
using namespace std;


void LLC::send_mem_req(bool op, LLC_DATA_ADDR addr, line_t data_line)
{
}

void LLC::get_mem_rsp(line_t &line)
{
}

void LLC::ctrl()
{
    bool is_rsp_to_get = 0;
    bool is_req_to_get = 0;
    if (is_rsp_to_get)
    {

        /// @TODO handle responses
        llc addr;
        bitset<LLC_REQS_BITS> reqs_hit_i;
        addr.breakdown(rsp_in.addr << OFFSET_BITS);
        switch (rsp_in.coh_msg)
        {
        case RSP_INV_ACK_SPDX:
        {
            reqs[reqs_hit_i].invack_cnt--;
            sharers_buf[reqs[reqs_hit_i].way] &= ~(1 << rsp_in.req_id); /// clears sharer bit

            if (reqs[reqs_hit_i].invack_cnt == 0)
            {
                switch (reqs[reqs_hit_i].state)
                {
                case LLC_SO:
                {
                    if (reqs[reqs_hit_i].msg == REQ_O)
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-800");
                        send_rsp_out(RSP_O, rsp_in.addr, 0, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    else
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-804");
                        send_rsp_out(RSP_Odata, rsp_in.addr, reqs[reqs_hit_i].line, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    state_buf[reqs[reqs_hit_i].way] = LLC_V;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                }
                break;
                case LLC_SV:
                {
                    if (reqs[reqs_hit_i].msg == REQ_WT || reqs[reqs_hit_i].msg == REQ_WTfwd)
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-813");
                        send_rsp_out(RSP_O, rsp_in.addr, 0, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    else if (reqs[reqs_hit_i].is_amo)
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-817");
                        calc_amo(lines_buf[reqs[reqs_hit_i].way], reqs[reqs_hit_i].line, req_in.coh_msg, req_in.word_mask);
                        send_rsp_out(RSP_WTdata, rsp_in.addr, reqs[reqs_hit_i].line, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    else
                    {
                        /// nothing
                    }
                    lines_buf[reqs[reqs_hit_i].way] = reqs[reqs_hit_i].line;
                    state_buf[reqs[reqs_hit_i].way] = LLC_V;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    if (recall_pending && rsp_in.addr == recall_addr)
                        recall_valid = true;
#ifdef LLC_DEBUG
                    dbg_recall_valid.write(recall_valid);
#endif
                }
                break;
                case LLC_SWB:
                {
                    HLS_DEFINE_PROTOCOL("send-mem-821");
                    send_mem_req(WRITE, rsp_in.addr, reqs[reqs_hit_i].hprot, reqs[reqs_hit_i].line);
                    state_buf[reqs[reqs_hit_i].way] = LLC_I;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    evict_inprogress = 0;
                }
                break;
                case LLC_SI:
                {
                    state_buf[reqs[reqs_hit_i].way] = LLC_I;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    evict_inprogress = 0;
                }
                break;
                default:
                {
                    /// nothing
                }
                break;
                }
            }
        }
        break;

        case RSP_RVK_O:
        {
            for (int i = 0; i < WORDS_PER_LINE; i++)
            {
                HLS_UNROLL_LOOP(ON, "rvk-wb");
                if (owners_buf[reqs[reqs_hit_i].way] & (1 << i) & rsp_in.word_mask)
                {
                    /// /// found a matching bit in mask
                    /// if (rsp_in.req_id.to_int() == lines_buf[reqs[reqs_hit_i].way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD).to_int()) /// if owner id == req id
                    /// {
                    lines_buf[reqs[reqs_hit_i].way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = rsp_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD); /// write back new data
                    owners_buf[reqs[reqs_hit_i].way] = owners_buf[reqs[reqs_hit_i].way] & (~(1 << i));                                                                         /// clear owner bit
                    dirty_bits_buf[reqs[reqs_hit_i].way] = 1;
                    sharers_buf[reqs[reqs_hit_i].way] |= 1 << rsp_in.req_id;
                    /// }
                }
            }
            switch (reqs[reqs_hit_i].state)
            {
            case LLC_OS:
            {
                if (owners_buf[reqs[reqs_hit_i].way] == 0)
                {
                    /// change state
                    state_buf[reqs[reqs_hit_i].way] = LLC_S;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    sharers_buf[reqs[reqs_hit_i].way] |= 1 << reqs[reqs_hit_i].req_id;
                }
            }
            break;
            case LLC_OV:
            {
                if (owners_buf[reqs[reqs_hit_i].way] == 0)
                {
                    if (reqs[reqs_hit_i].msg == REQ_WT)
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-813");
                        send_rsp_out(RSP_O, rsp_in.addr, 0, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    else if (reqs[reqs_hit_i].is_amo)
                    {
                        HLS_DEFINE_PROTOCOL("send-rsp-817");
                        calc_amo(lines_buf[reqs[reqs_hit_i].way], reqs[reqs_hit_i].line, req_in.coh_msg, req_in.word_mask);
                        send_rsp_out(RSP_WTdata, rsp_in.addr, reqs[reqs_hit_i].line, reqs[reqs_hit_i].req_id, reqs[reqs_hit_i].req_id, 0, 0, reqs[reqs_hit_i].word_mask);
                    }
                    else
                    {
                        /// nothing
                    }
                    state_buf[reqs[reqs_hit_i].way] = LLC_V;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    if (recall_pending && rsp_in.addr == recall_addr)
                        recall_valid = true;
#ifdef LLC_DEBUG
                    dbg_recall_valid.write(recall_valid);
#endif
                }
            }
            break;
            case LLC_OWB:
            {
                if (owners_buf[reqs[reqs_hit_i].way] == 0)
                {
                    /// wb and goto I
                    {
                        HLS_DEFINE_PROTOCOL("send-mem-875");
                        send_mem_req(WRITE, rsp_in.addr, reqs[reqs_hit_i].hprot, lines_buf[reqs[reqs_hit_i].way]);
                    }
                    state_buf[reqs[reqs_hit_i].way] = LLC_I;
                    reqs[reqs_hit_i].state = LLC_I;
                    reqs_cnt++;
                    evict_inprogress = 0;
                }
            }
            break;
            default:
            {
                /// nothing
            }
            break;
            }
        }
        break;
        default:
            break;
        }
    }

    /// Process new request
    else if (is_req_to_get)
    {

        llc_tag_t addr;
        // bitset<LLC_REQS_BITS> reqs_hit_i, reqs_empty_i;
        llc_tag_t evict_addr_br;
        evict_addr_br.breakdown(addr_evict_real);
        addr.breakdown(req_in.addr << OFFSET_BITS);
        set_conflict = reqs_peek_req(evict_addr_br, reqs_hit_i) || reqs_peek_req(addr, reqs_empty_i) || (reqs_cnt == 0);
        reqs_lookup(addr, reqs_hit_i);
        evict_stall = evict_inprogress;

        if (evict_stall || set_conflict) /// optimize
        {
            /// attempt to resolve requests that hit in transient state
            switch (req_in.coh_msg)
            {
            case REQ_S:
            {
                /// if (sharers_buf[reqs[reqs_hit_i].way] & (1 << req_in.req_id)) { /// only allow relaxed rsp_s if private cache has a pending inv
                ///         switch (reqs[reqs_hit_i].state) {
                ///                 case LLC_SI: /// to solve reqs vs inv deadlock on LLC_SX due to silent ecivtion
                ///                 {
                ///                         HLS_DEFINE_PROTOCOL("deadlock-solver-1");
                ///                         send_rsp_out(RSP_S, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                ///                         send_fwd_out(FWD_INV_SPDX, req_in.addr, 0, req_in.req_id, WORD_MASK_ALL);
                ///                         reqs[reqs_hit_i].invack_cnt++;
                ///                         evict_stall = 0;
                ///                 }
                ///                 break;
                ///                 case LLC_SO: /// to solve reqs vs inv deadlock on LLC_SX due to silent ecivtion
                ///                 {
                ///                         HLS_DEFINE_PROTOCOL("deadlock-solver-2");
                ///                         send_rsp_out(RSP_S, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                ///                         send_fwd_out(FWD_INV_SPDX, req_in.addr, 0, req_in.req_id, WORD_MASK_ALL);
                ///                         reqs[reqs_hit_i].invack_cnt++;
                ///                         set_conflict = 0;
                ///                 }
                ///                 break;
                ///                 default:
                ///                 break;
                ///         }
                /// }
            }
            break;
            default:
                break;
            }
            /// failed to resolve request
            if (set_conflict)
                llc_req_conflict = req_in;
            if (evict_stall)
                llc_req_stall = req_in;
        }
        else
        {

            if (evict)
            {
                LLC_EVICT;

                fcs_prio_buf[way] = 0;

                if (way == evict_ways_buf)
                {
                    update_evict_ways = true;
                    evict_ways_buf++;
                }

                switch (state_buf[way])
                {
                /// here we made sure that this set is all in stable state
                /// because any request conflict in unstable buffer has been blocked by set_conflict
                case LLC_V:
                {
                    if (owners_buf[way] == 0)
                    {
                        if (dirty_bits_buf[way])
                        {
                            HLS_DEFINE_PROTOCOL("send_mem_req-2");
                            send_mem_req(WRITE, addr_evict, hprots_buf[way], lines_buf[way]);
                        }
                        state_buf[way] = LLC_I;
                    }
                    else
                    {
                        (void)send_fwd_with_owner_mask(FWD_RVK_O, addr_evict, req_in.req_id, owners_buf[way], lines_buf[way]);
                        fill_reqs(FWD_RVK_O, req_in.req_id, evict_addr_br, 0, way, LLC_OWB, hprots_buf[way], 0, lines_buf[way], owners_buf[way], reqs_empty_i); /// save this request in reqs buffer
                        evict_stall = true;
                        evict_inprogress = true;
                    }
                }
                break;
                case LLC_S:
                {
                    int cnt = send_inv_with_sharer_list(addr_evict, sharers_buf[way]);
                    if (dirty_bits_buf[way])
                        fill_reqs(FWD_INV_SPDX, req_in.req_id, evict_addr_br, 0, way, LLC_SWB, hprots_buf[way], 0, lines_buf[way], owners_buf[way], reqs_empty_i); /// save this request in reqs buffer
                    else
                        fill_reqs(FWD_INV_SPDX, req_in.req_id, evict_addr_br, 0, way, LLC_SI, hprots_buf[way], 0, lines_buf[way], owners_buf[way], reqs_empty_i); /// save this request in reqs buffer
                    reqs[reqs_empty_i].invack_cnt = cnt;
                    evict_stall = true;
                    evict_inprogress = true;
                }
                break;
                default:
                    break;
                }
            }
            /// set pending eviction and skip request
            if (evict_stall)
            {
                llc_req_stall = req_in;
            }
            else if (is_amo(req_in.coh_msg))
            {
                /// if in owned state
                dirty_bits_buf[way] = 1;
                switch (state_buf[way])
                {
                case LLC_I:
                {
                    /// if in invalid, read out data and perform amo
                    HLS_DEFINE_PROTOCOL("send_mem_req-948");
                    send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                    get_mem_rsp(lines_buf[way]);
                    calc_amo(lines_buf[way], req_in.line, req_in.coh_msg, req_in.word_mask);
                    send_rsp_out(RSP_WTdata, req_in.addr, req_in.line, req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask); /// send old data out
                    hprots_buf[way] = req_in.hprot;
                    tag_buf[way] = line_br.tag;
                    dirty_bits_buf[way] = 0;
                    state_buf[way] = LLC_V;
                }
                break;
                case LLC_V:
                {
                    /// if amo is on not owned word, we are lucky
                    if ((owners_buf[way] & req_in.word_mask) == 0)
                    {
                        calc_amo(lines_buf[way], req_in.line, req_in.coh_msg, req_in.word_mask);
                        send_rsp_out(RSP_WTdata, req_in.addr, req_in.line, req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask); /// send old data out
                    }
                    else
                    {
                        /// send rvko
                        /// @TODO this might need thinking
                        send_fwd_with_owner_mask(FWD_RVK_O, req_in.addr, req_in.req_id, req_in.word_mask, 0);
                        fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_OV, hprots_buf[way], 0, req_in.line, owners_buf[way], reqs_empty_i); /// save this request in reqs buffer
                        reqs[reqs_empty_i].is_amo = true;
                    }
                }
                break;
                case LLC_S:
                {
                    int cnt = send_inv_with_sharer_list(req_in.addr, sharers_buf[way]);
                    fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_SV, req_in.hprot, 0, req_in.line, req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                    reqs[reqs_empty_i].invack_cnt = cnt;
                    reqs[reqs_empty_i].is_amo = true;
                }
                break;

                default:
                    break;
                }
            }
            else
            {

                switch (req_in.coh_msg)
                {

                case REQ_V:

                    /// LLC_REQV;
                    switch (state_buf[way])
                    {
                    case LLC_I:
                    {
                        HLS_DEFINE_PROTOCOL("send_mem_req-948");
                        send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                        get_mem_rsp(lines_buf[way]);
                        send_rsp_out(RSP_V, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 0;
                        state_buf[way] = LLC_V;
                        owners_buf[way] = 0;
                    }
                    break;
                    case LLC_V:
                    {
                        /// REQV_IV;

                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        if (word_owner_mask)
                        {
                            /// if owner exist
                            send_fwd_with_owner_mask(FWD_REQ_V, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way]);
                        }

                        if (word_no_owner_mask)
                        {
                            /// if left over words exist
                            HLS_DEFINE_PROTOCOL("send_rsp_974");
                            send_rsp_out(RSP_V, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                        }
                    }
                    break;
                    case LLC_S:
                    {
                        /// REQV_S;
                        HLS_DEFINE_PROTOCOL("send_rsp_982");
                        send_rsp_out(RSP_V, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                    }
                    break;
                    default:
                        GENERIC_ASSERT;
                    }

                    break;

                case REQ_S:
                    /// LLC_REQS;

                    switch (state_buf[way])
                    {

                    case LLC_I:
                    {
                        /// REQS_IV;
                        HLS_DEFINE_PROTOCOL("send_mem_req_1000");
                        send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                        get_mem_rsp(lines_buf[way]);
                        send_rsp_out(RSP_S, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 0;
                        state_buf[way] = LLC_S;
                        sharers_buf[way] = 1 << req_in.req_id;
                    }
                    break;
                    case LLC_V:
                    {
                        /// REQS_IV;

                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        other_owner = 0;
                        if (word_owner_mask)
                        {
                            other_owner = send_fwd_with_owner_mask(FWD_REQ_S, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way]);
                            if (other_owner)
                            {
                                fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_OS, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                            }
                        }
                        /// send rsp for those words without owners
                        if (word_no_owner_mask)
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1027");
                            send_rsp_out(RSP_S, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                            /// if other_owner is 0, then it means no transient state in reqs, and we can change state and shares here
                            if (!other_owner)
                            {
                                state_buf[way] = LLC_S;
                                sharers_buf[way] = 1 << req_in.req_id;
                            }
                        }
                    }
                    break;

                    case LLC_S:
                    {
                        /// REQS_S;
                        HLS_DEFINE_PROTOCOL("send_rsp_1039");
                        send_rsp_out(RSP_S, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        sharers_buf[way] = sharers_buf[way] | (1 << req_in.req_id);
                    }
                    break;

                    default:
                        GENERIC_ASSERT;
                    }

                    break;

                case REQ_O:

                    /// LLC_REQO;

                    switch (state_buf[way])
                    {

                    case LLC_I:
                    {

                        if (req_in.word_mask != WORD_MASK_ALL)
                        {
                            HLS_DEFINE_PROTOCOL("send_mem_req_1059");
                            send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                            get_mem_rsp(lines_buf[way]);
                        }
                        owners_buf[way] = req_in.word_mask;
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                            }
                        }
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 0;
                        state_buf[way] = LLC_V;
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1078");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                    }
                    break;
                    case LLC_V:
                    {
                        /// REQO_V;
                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        if (word_owner_mask)
                        {
                            send_fwd_with_owner_mask(FWD_REQ_O, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way]);
                        }
                        if (word_no_owner_mask)
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1100");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                        }
                        /// update owner
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                            }
                        }
                        owners_buf[way] |= req_in.word_mask;
                    }
                    break;

                    case LLC_S:
                    {
                        /// assume word_mask > 0
                        /// REQO_S;
                        /// invalidate
                        int cnt = 0;
                        /// special case, cannot call inline helper
                        for (int i = 0; i < MAX_N_L2; i++)
                        {
                            HLS_DEFINE_PROTOCOL("send_fwd_1116");
                            if (sharers_buf[way] & (1 << i))
                            {
                                if (req_in.req_id == i)
                                {
                                    /// upgrade
                                }
                                else
                                {
                                    send_fwd_out(FWD_INV_SPDX, req_in.addr, req_in.req_id, i, req_in.word_mask);
                                    cnt++;
                                }
                            }
                            wait();
                        }

                        state_buf[way] = LLC_V;
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                        }
                        owners_buf[way] = req_in.word_mask;

                        if (cnt == 0)
                        {
                            /// only upgrade
                            /// @TODO send original data
                            HLS_DEFINE_PROTOCOL("send_rsp_1198");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                        else
                        {
                            /// wait for invack
                            fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_SO, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                            reqs[reqs_empty_i].invack_cnt = cnt;
                        }
                    }
                    break;

                    default:
                        GENERIC_ASSERT;
                    }

                    break;

                case REQ_WT:

                    /// LLC_REQWT;

                    switch (state_buf[way])
                    {

                    case LLC_I:
                    {
                        {
                            HLS_DEFINE_PROTOCOL("send_men_1140");
                            send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                            get_mem_rsp(lines_buf[way]);
                        }

                        /// write new data
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask-1151");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                            }
                        }
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 1;
                        state_buf[way] = LLC_V;
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1157");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                    }
                    break;
                    case LLC_V:
                    {
                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        if (word_owner_mask)
                        {
                            send_fwd_with_owner_mask(FWD_REQ_O, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way]);
                        }
                        if (word_no_owner_mask)
                        {
                            HLS_DEFINE_PROTOCOL("req_wt send rsp_o in llc_v");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                        }
                        /// write new data
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask-1176");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                                owners_buf[way] = owners_buf[way] & (~(1 << i)); /// clear owner bit
                            }
                        }
                        dirty_bits_buf[way] = 1;
                    }
                    break;
                    case LLC_S:
                    {
                        /// REQWT_S;
                        /// invalidate
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask-1176");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                            }
                        }
                        int cnt = send_inv_with_sharer_list(req_in.addr, sharers_buf[way]);
                        fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_SV, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                        reqs[reqs_empty_i].invack_cnt = cnt;
                        dirty_bits_buf[way] = 1;
                    }
                    break;

                    default:
                        GENERIC_ASSERT;
                    }

                    break;

                case REQ_WTfwd:

                    fcs_prio_buf[way] = 1;

                    switch (state_buf[way])
                    {

                    case LLC_I:
                    {
                        {
                            HLS_DEFINE_PROTOCOL("send_men_req_wtfwd");
                            send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                            get_mem_rsp(lines_buf[way]);
                        }

                        /// write new data
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask-1151");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                            }
                        }
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 1;
                        state_buf[way] = LLC_V;
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1157");
                            send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                    }
                    break;
                    case LLC_V:
                    {
                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        if (word_owner_mask)
                        {
                            send_fwd_with_owner_mask_data(FWD_WTfwd, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way], req_in.line);
                        }
                        if (word_no_owner_mask)
                        {
                            for (int i = 0; i < WORDS_PER_LINE; i++)
                            {
                                HLS_UNROLL_LOOP(ON, "set-ownermask-1176");
                                if (word_no_owner_mask & (1 << i))
                                {
                                    lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                                }
                            }
                            {
                                HLS_DEFINE_PROTOCOL("req_wtfwd send rsp o when llc_v");
                                send_rsp_out(RSP_O, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                            }
                        }
                        dirty_bits_buf[way] = 1;
                    }
                    break;
                    case LLC_S:
                    {
                        /// REQWTfwd_S;
                        /// invalidate
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask-1176");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD);
                            }
                        }
                        int cnt = send_inv_with_sharer_list(req_in.addr, sharers_buf[way]);
                        fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_SV, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                        reqs[reqs_empty_i].invack_cnt = cnt;
                        dirty_bits_buf[way] = 1;
                    }
                    break;

                    default:
                        GENERIC_ASSERT;
                    }

                    break;

                case REQ_Odata:

                    /// Read with ReqOdata
                    if (req_in.hprot == INSTR)
                    {
                        fcs_prio_buf[way] = 1;
                    }

                    /// LLC_REQO;

                    switch (state_buf[way])
                    {

                    case LLC_I:
                    {
                        {
                            HLS_DEFINE_PROTOCOL("send_men_1219");
                            send_mem_req(READ, req_in.addr, req_in.hprot, 0);
                            get_mem_rsp(lines_buf[way]);
                        }
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1238");
                            send_rsp_out(RSP_Odata, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                        owners_buf[way] = req_in.word_mask;
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                            }
                        }
                        hprots_buf[way] = req_in.hprot;
                        tag_buf[way] = line_br.tag;
                        dirty_bits_buf[way] = 0;
                        state_buf[way] = LLC_V;
                    }
                    break;
                    case LLC_V:
                    {
                        /// REQO_V;
                        word_owner_mask = owners_buf[way] & req_in.word_mask;
                        word_no_owner_mask = req_in.word_mask & ~owners_buf[way];
                        if (word_owner_mask)
                        {
                            send_fwd_with_owner_mask(FWD_REQ_Odata, req_in.addr, req_in.req_id, word_owner_mask, lines_buf[way]);
                        }
                        if (word_no_owner_mask)
                        {
                            HLS_DEFINE_PROTOCOL("send_rsp_1249");
                            send_rsp_out(RSP_Odata, req_in.addr, lines_buf[way], req_in.req_id, req_in.req_id, 0, 0, word_no_owner_mask);
                        }
                        /// update owner
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                            {
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                            }
                        }
                        owners_buf[way] |= req_in.word_mask;
                    }
                    break;

                    case LLC_S:
                    {
                        /// REQOdata_S;
                        /// invalidate
                        int cnt = 0;
                        line_t temp = lines_buf[way];
                        /// @TODO optimize
                        /// special case, cannot call inline helper
                        for (int i = 0; i < MAX_N_L2; i++)
                        {
                            HLS_DEFINE_PROTOCOL("send_fwd_1116");
                            if (sharers_buf[way] & (1 << i))
                            {
                                if (req_in.req_id == i)
                                {
                                    /// upgrade
                                    sharers_buf[way] &= ~(1 << i);
                                }
                                else
                                {
                                    send_fwd_out(FWD_INV_SPDX, req_in.addr, req_in.req_id, i, req_in.word_mask);
                                    cnt++;
                                }
                            }
                            wait();
                        }

                        state_buf[way] = LLC_V;
                        for (int i = 0; i < WORDS_PER_LINE; i++)
                        {
                            HLS_UNROLL_LOOP(ON, "set-ownermask");
                            if (req_in.word_mask & (1 << i))
                                lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD) = req_in.req_id;
                        }
                        owners_buf[way] = req_in.word_mask;

                        if (cnt == 0)
                        {
                            /// only upgrade
                            HLS_DEFINE_PROTOCOL("send_rsp_1198");
                            send_rsp_out(RSP_Odata, req_in.addr, temp, req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                        }
                        else
                        {
                            /// wait for invack
                            fill_reqs(req_in.coh_msg, req_in.req_id, addr, 0, way, LLC_SO, req_in.hprot, 0, temp, req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
                            reqs[reqs_empty_i].invack_cnt = cnt;
                        }
                    }
                    break;

                    default:
                        GENERIC_ASSERT;
                    }
                    break;

                case REQ_WB:
                    word_owner_mask = owners_buf[way] & req_in.word_mask;
                    /// send response
                    {
                        HLS_DEFINE_PROTOCOL("send_rsp_1330");
                        send_rsp_out(RSP_WB_ACK, req_in.addr, 0, req_in.req_id, req_in.req_id, 0, 0, req_in.word_mask);
                    }
                    if (word_owner_mask == 0)
                        break; /// in stable states, no owner, no need to do anything
                    for (int i = 0; i < WORDS_PER_LINE; i++)
                    {
                        HLS_UNROLL_LOOP(ON, "check-wb-ownermask");
                        if (word_owner_mask & (1 << i))
                        {
                            /// found a mathcing bit in mask
                            if (req_in.req_id == lines_buf[way].range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD)) /// if owner id == req id
                            {
                                lines_buf[way].range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD) = req_in.line.range((i + 1) * BITS_PER_WORD - 1, i * BITS_PER_WORD); /// write back new data
                                owners_buf[way] = owners_buf[way] & (~(1 << i));                                                                                          /// clear owner bit
                                dirty_bits_buf[way] = 1;
                            }
                        }
                    }
                    break;

                default:
                    GENERIC_ASSERT;
                }
            }
        }
    }
}