#include "classes.hpp"
using namespace std;

void LLC::read_set(const llc_addr_t base, const llc_way_t way_offset)
{
    /// #if LLC_WAYS == 32
    ///     for (int i = 0; i < LLC_LOOKUP_WAYS; i++) {
    /// #else
    ///     for (int i = 0; i < LLC_WAYS; i++) {
    /// #endif
    ///         HLS_UNROLL_LOOP(ON);

    ///         tag_buf[i + way_offset]       = tags[base + i + way_offset];
    ///         state_buf[i + way_offset]     = states[base + i + way_offset];
    ///         hprots_buf[i + way_offset]     = hprots[base + i + way_offset];
    ///         lines_buf[i + way_offset]      = lines[base + i + way_offset];
    ///         owners_buf[i + way_offset]     = owners[base + i + way_offset];
    ///         sharers_buf[i + way_offset]   = sharers[base + i + way_offset];
    ///         dirty_bits_buf[i + way_offset] = dirty_bits[base + i + way_offset];
    ///     }

    tag_buf[0 + way_offset] = tags.port2[0][base + 0 + way_offset];
    state_buf[0 + way_offset] = states.port2[0][base + 0 + way_offset];
    hprots_buf[0 + way_offset] = hprots.port2[0][base + 0 + way_offset];
    lines_buf[0 + way_offset] = lines.port2[0][base + 0 + way_offset];
    owners_buf[0 + way_offset] = owners.port2[0][base + 0 + way_offset];
    sharers_buf[0 + way_offset] = sharers.port2[0][base + 0 + way_offset];
    dirty_bits_buf[0 + way_offset] = dirty_bits.port2[0][base + 0 + way_offset];
    fcs_prio_buf[0 + way_offset] = fcs_prio.port2[0][base + 0 + way_offset];

    tag_buf[1 + way_offset] = tags.port3[0][base + 1 + way_offset];
    state_buf[1 + way_offset] = states.port3[0][base + 1 + way_offset];
    hprots_buf[1 + way_offset] = hprots.port3[0][base + 1 + way_offset];
    lines_buf[1 + way_offset] = lines.port3[0][base + 1 + way_offset];
    owners_buf[1 + way_offset] = owners.port3[0][base + 1 + way_offset];
    sharers_buf[1 + way_offset] = sharers.port3[0][base + 1 + way_offset];
    dirty_bits_buf[1 + way_offset] = dirty_bits.port3[0][base + 1 + way_offset];
    fcs_prio_buf[1 + way_offset] = fcs_prio.port3[0][base + 1 + way_offset];

    tag_buf[2 + way_offset] = tags.port4[0][base + 2 + way_offset];
    state_buf[2 + way_offset] = states.port4[0][base + 2 + way_offset];
    hprots_buf[2 + way_offset] = hprots.port4[0][base + 2 + way_offset];
    lines_buf[2 + way_offset] = lines.port4[0][base + 2 + way_offset];
    owners_buf[2 + way_offset] = owners.port4[0][base + 2 + way_offset];
    sharers_buf[2 + way_offset] = sharers.port4[0][base + 2 + way_offset];
    dirty_bits_buf[2 + way_offset] = dirty_bits.port4[0][base + 2 + way_offset];
    fcs_prio_buf[2 + way_offset] = fcs_prio.port4[0][base + 2 + way_offset];

    tag_buf[3 + way_offset] = tags.port5[0][base + 3 + way_offset];
    state_buf[3 + way_offset] = states.port5[0][base + 3 + way_offset];
    hprots_buf[3 + way_offset] = hprots.port5[0][base + 3 + way_offset];
    lines_buf[3 + way_offset] = lines.port5[0][base + 3 + way_offset];
    owners_buf[3 + way_offset] = owners.port5[0][base + 3 + way_offset];
    sharers_buf[3 + way_offset] = sharers.port5[0][base + 3 + way_offset];
    dirty_bits_buf[3 + way_offset] = dirty_bits.port5[0][base + 3 + way_offset];
    fcs_prio_buf[3 + way_offset] = fcs_prio.port5[0][base + 3 + way_offset];
}


void LLC::send_rsp_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, cache_id_t req_id,
                              cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t word_offset, word_mask_t word_mask)
{
    SEND_RSP_OUT;
    llc_rsp_out_t<CACHE_ID_WIDTH> rsp_out;
    rsp_out.coh_msg = coh_msg;
    rsp_out.addr = addr;
    rsp_out.line = line;
    rsp_out.req_id = req_id;
    rsp_out.dest_id = dest_id;
    rsp_out.invack_cnt = invack_cnt;
    rsp_out.word_offset = word_offset;
    rsp_out.word_mask = word_mask;
    while (!llc_rsp_out.nb_can_put())
        wait();
    llc_rsp_out.nb_put(rsp_out);
}

void LLC::send_fwd_out(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, cache_id_t dest_id, word_mask_t word_mask)
{
    SEND_FWD_OUT;
    llc_fwd_out_t fwd_out;

    fwd_out.coh_msg = coh_msg;
    fwd_out.addr = addr;
    fwd_out.req_id = req_id;
    fwd_out.dest_id = dest_id;
    fwd_out.word_mask = word_mask;
    while (!llc_fwd_out.nb_can_put())
        wait();
    llc_fwd_out.nb_put(fwd_out);
}

void LLC::send_dma_rsp_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, llc_coh_dev_id_t req_id,
                                  cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t word_offset)
{
    SEND_DMA_RSP_OUT;
    llc_rsp_out_t<LLC_COH_DEV_ID_WIDTH> rsp_out;
    rsp_out.coh_msg = coh_msg;
    rsp_out.addr = addr;
    rsp_out.line = line;
    rsp_out.req_id = req_id;
    rsp_out.dest_id = dest_id;
    rsp_out.invack_cnt = invack_cnt;
    rsp_out.word_offset = word_offset;
    while (!llc_dma_rsp_out.nb_can_put())
        wait();
    llc_dma_rsp_out.nb_put(rsp_out);
}

void LLC::send_fwd_out_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, cache_id_t dest_id, word_mask_t word_mask, line_t data)
{
    SEND_FWD_OUT;
    llc_fwd_out_t fwd_out;

    fwd_out.coh_msg = coh_msg;
    fwd_out.addr = addr;
    fwd_out.req_id = req_id;
    fwd_out.dest_id = dest_id;
    fwd_out.word_mask = word_mask;
    fwd_out.line = data;
    while (!llc_fwd_out.nb_can_put())
        wait();
    llc_fwd_out.nb_put(fwd_out);
}

inline bool LLC::send_fwd_with_owner_mask(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, word_mask_t word_mask, line_t data)
{
    fwd_coal_send_count = 0;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (word_mask & (1 << i))
        {
            int owner = data.range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD).to_int();
            if (owner != req_id || coh_msg == FWD_RVK_O)
            { /// skip if requestor already owns word, still send if revoke
                bool coal = 0;
                for (int j = 0; j < fwd_coal_send_count; j++)
                {
                    if (fwd_coal_temp_dest[j] == owner)
                    {
                        fwd_coal_word_mask[j] = fwd_coal_word_mask[j] | (1 << i);
                        coal = true;
                        break;
                    }
                    wait();
                }
                /// new forward dest
                if (!coal)
                {
                    fwd_coal_temp_dest[fwd_coal_send_count] = owner;
                    fwd_coal_word_mask[fwd_coal_send_count] = 0 | (1 << i);
                    fwd_coal_send_count = fwd_coal_send_count + 1;
                }
            }
        }
        wait();
    }
    for (int i = 0; i < fwd_coal_send_count; i++)
    {
        HLS_DEFINE_PROTOCOL();
        send_fwd_out(coh_msg, addr, req_id, fwd_coal_temp_dest[i], fwd_coal_word_mask[i]);
        fwd_coal_word_mask[i] = 0;
        wait();
    }
    return fwd_coal_send_count > 0;
}

inline bool LLC::send_fwd_with_owner_mask_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, word_mask_t word_mask, line_t data, line_t data_out)
{
    fwd_coal_send_count = 0;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (word_mask & (1 << i))
        {
            int owner = data.range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD).to_int();
            if (owner != req_id || coh_msg == FWD_RVK_O)
            { /// skip if requestor already owns word, still send if revoke
                bool coal = 0;
                for (int j = 0; j < fwd_coal_send_count; j++)
                {
                    if (fwd_coal_temp_dest[j] == owner)
                    {
                        fwd_coal_word_mask[j] = fwd_coal_word_mask[j] | (1 << i);
                        coal = true;
                        break;
                    }
                    wait();
                }
                /// new forward dest
                if (!coal)
                {
                    fwd_coal_temp_dest[fwd_coal_send_count] = owner;
                    fwd_coal_word_mask[fwd_coal_send_count] = 0 | (1 << i);
                    fwd_coal_send_count = fwd_coal_send_count + 1;
                }
            }
        }
        wait();
    }
    for (int i = 0; i < fwd_coal_send_count; i++)
    {
        send_fwd_out_data(coh_msg, addr, req_id, fwd_coal_temp_dest[i], fwd_coal_word_mask[i], data_out);
        fwd_coal_word_mask[i] = 0;
        wait();
    }
    return fwd_coal_send_count > 0;
}

inline int LLC::send_inv_with_sharer_list(line_addr_t addr, sharers_t sharer_list)
{
    HLS_DEFINE_PROTOCOL("inv_all_sharers");
    int cnt = 0;
    for (int i = 0; i < MAX_N_L2; i++)
    {
        if (sharer_list & (1 << i))
        {
            send_fwd_out(FWD_INV_SPDX, addr, 0, i, WORD_MASK_ALL);
            cnt++;
        }
        wait();
    }
    return cnt;
}

/// write reqs buf
void LLC::fill_reqs(mix_msg_t msg, cache_id_t req_id, addr_breakdown_llc_t addr_br, llc_tag_t tag_estall, llc_way_t way_hit,
                    llc_unstable_state_t state, hprot_t hprot, word_t word, line_t line, word_mask_t word_mask, bitset<LLC_REQS_BITS> reqs_i)
{
    LLC_FILL_REQS;

    reqs[reqs_i].msg = msg;
    reqs[reqs_i].tag = addr_br.tag;
    reqs[reqs_i].tag_estall = tag_estall;
    reqs[reqs_i].set = addr_br.set;
    reqs[reqs_i].way = way_hit;
    reqs[reqs_i].w_off = addr_br.w_off;
    reqs[reqs_i].b_off = addr_br.b_off;
    reqs[reqs_i].state = state;
    reqs[reqs_i].hprot = hprot;
    reqs[reqs_i].invack_cnt = MAX_N_L2;
    reqs[reqs_i].word = word;
    reqs[reqs_i].line = line;
    reqs[reqs_i].word_mask = word_mask;
    reqs[reqs_i].req_id = req_id;
    reqs[reqs_i].is_amo = 0;
    reqs_cnt--;
}



bool LLC::reqs_peek_req(addr_breakdown_llc_t br, bitset<LLC_REQS_BITS> &reqs_empty_i)
{
    LLC_REQS_PEEK_REQ;

    set_conflict = 0;

    for (unsigned int i = 0; i < LLC_N_REQS; ++i)
    {
        LLC_REQS_PEEK_REQ_LOOP;

        if (reqs[i].state == LLC_I)
            reqs_empty_i = i;

        if (reqs[i].tag == br.tag && reqs[i].set == br.set && reqs[i].state != LLC_I)
            set_conflict = true;
    }

#ifdef LLC_DEBUG
    /// @TODO
    /// peek_reqs_i_dbg.write(reqs_i);
#endif

    return set_conflict;
}

/*
 * Processes
 */

void LLC::ctrl()
{
    /// -----------------------------
    /// RESET
    /// -----------------------------

    /// Reset all signals and channels
    {
        HLS_DEFINE_PROTOCOL("reset-io");

        this->reset_io();
    }

    /// Reset state memory
    {
        HLS_DEFINE_PROTOCOL("reset_state-1");

        this->reset_state();
    }

    while (true)
    {

        bool is_rst_to_resume = 0;
        bool is_flush_to_resume = 0;
        bool is_rst_to_get = 0;
        bool is_rsp_to_get = 0;
        bool is_req_to_get = 0;
        bool is_dma_read_to_resume = 0;
        bool is_dma_write_to_resume = 0;
        bool is_dma_req_to_get = 0;

        bool rst_in = 0;
        llc_rsp_in_t rsp_in;
        llc_req_in_t<CACHE_ID_WIDTH> req_in;

        bool can_get_rst_tb = 0;
        bool can_get_rsp_in = 0;
        bool can_get_req_in = 0;
        bool can_get_dma_in = 0;

        bool update_evict_ways = 0;

        bool look;
        bool other_owner;
        line_breakdown_t<llc_tag_t, llc_set_t> line_br;
        llc_set_t set;
        llc_way_t way;
        bool evict;
        llc_addr_t base;
        llc_addr_t llc_addr;
        line_addr_t addr_evict;
        addr_t addr_evict_real;
        word_mask_t word_owner_mask;
        word_mask_t word_no_owner_mask;

        /// -----------------------------
        /// Check input channels
        /// -----------------------------

        {
            HLS_DEFINE_PROTOCOL("proto-llc-io-check");
            bool do_get_req = 0;
            bool do_get_dma_req = 0;

            can_get_rst_tb = llc_rst_tb.nb_can_get();
            can_get_rsp_in = llc_rsp_in.nb_can_get();
            can_get_req_in = llc_req_in.nb_can_get();
            can_get_dma_in = llc_dma_req_in.nb_can_get();

            if (can_get_rsp_in)
            {
                /// Response
                is_rsp_to_get = true;
            }
            else if (recall_pending)
            {
                if (!recall_valid)
                {
                    /// Response (could be related to the recall or not)
                    if (can_get_rsp_in)
                    {
                        is_rsp_to_get = true;
                    }
                }
                else
                {
                    if (dma_read_pending)
                        is_dma_read_to_resume = true;
                    else if (dma_write_pending)
                        is_dma_write_to_resume = true;
                }
            }
            else if (rst_stall)
            {
                /// Pending reset
                is_rst_to_resume = true;
            }
            else if (flush_stall)
            {
                /// Pending flush
                is_flush_to_resume = true;
            }
            else if (can_get_rst_tb && !dma_read_pending && !dma_write_pending)
            {
                /// Reset or flush
                is_rst_to_get = true;
            }
            else if (can_get_req_in || evict_stall || set_conflict)
            {
                if (evict_stall)
                {
                    req_in = llc_req_stall;
                }
                else if (set_conflict)
                {
                    req_in = llc_req_conflict;
                }
                else
                {
                    do_get_req = true;
                }
                is_req_to_get = true;
            }
            else if (dma_read_pending)
            {
                /// Pending DMA read

                is_dma_read_to_resume = true;
            }
            else if (dma_write_pending)
            {
                /// Pending DMA write

                if (can_get_dma_in)
                {
                    is_dma_write_to_resume = true;
                    do_get_dma_req = true;
                }
            }
            else if (can_get_dma_in)
            {
                /// New DMA request
                is_dma_req_to_get = true;
                do_get_dma_req = true;
            }

            if (is_rsp_to_get)
            {
                llc_rsp_in.nb_get(rsp_in);
            }

            if (is_rst_to_get)
            {
                llc_rst_tb.nb_get(rst_in);
            }

            if (do_get_req)
            {
                llc_req_in.nb_get(req_in);
            }

            if (do_get_dma_req)
            {
                llc_dma_req_in.nb_get(dma_req_in);
            }
        }


        /// -----------------------------
        /// Lookup cache
        /// -----------------------------
        look = is_flush_to_resume || is_rsp_to_get || is_req_to_get || is_dma_req_to_get || (is_dma_read_to_resume && !recall_pending) || (is_dma_write_to_resume && !recall_pending);

        /// Pick right set

        if (is_flush_to_resume)
        {
            set = rst_flush_stalled_set;
            rst_flush_stalled_set++;
            if (rst_flush_stalled_set == 0)
            {
                rst_stall = 0;
                flush_stall = 0;
            }
        }
        else if (is_rst_to_resume)
        {
            set = rst_flush_stalled_set;
            /// Update current set
            rst_flush_stalled_set++;
            if (rst_flush_stalled_set == 0)
            {
                rst_stall = 0;
                flush_stall = 0;
            }
        }
        else if (is_rsp_to_get)
        {
            line_br.llc_line_breakdown(rsp_in.addr);
            set = line_br.set;
        }
        else if (is_req_to_get)
        {
            line_br.llc_line_breakdown(req_in.addr);
            set = line_br.set;
        }
        else if (is_dma_req_to_get || is_dma_read_to_resume || is_dma_write_to_resume)
        {
            if (is_dma_req_to_get)
                dma_addr = dma_req_in.addr;

            line_br.llc_line_breakdown(dma_addr);
            set = line_br.set;
        }

        /// Compute llc_address based on set
        base = set << LLC_WAY_BITS;

        /// Read all ways from set into buffer
        if (look)
        {

            HLS_DEFINE_PROTOCOL("read-cache");

            wait();
            read_set(base, 0);
            evict_ways_buf = evict_ways.port2[0][set];
#if LLC_WAYS == 32
            wait();
            read_set(base, 1);
#endif

#ifdef LLC_DEBUG
            dbg_evict_ways_buf = evict_ways_buf;
#endif
        }

        /// Select select way and determine potential eviction
        lookup(line_br.tag, way, evict);

        /// Compute llc_address based on selected way
        llc_addr = base + way;
        /// Compute memory address to use in case of eviction
        addr_evict = (tag_buf[way] << LLC_SET_BITS) + set;
        addr_evict_real = addr_evict << OFFSET_BITS;

        /// -----------------------------
        /// Process current request
        /// -----------------------------
        /// handle reset flush
        if (is_flush_to_resume)
        {
            /// partial flush (only VALID DATA lines)
            for (int way = 0; way < LLC_WAYS; way++)
            {
                line_addr_t line_addr = (tag_buf[way] << LLC_SET_BITS) | (set);

                if (state_buf[way] == LLC_V && hprots_buf[way] == DATA)
                {
                    HLS_DEFINE_PROTOCOL("is_flush_to_resume");
                    if (dirty_bits_buf[way])
                    {
                        send_mem_req(WRITE, line_addr, hprots_buf[way], lines_buf[way]);
                    }

                    wait();

                    llc_addr_t llc_addr = base + way;

                    states.port1[0][llc_addr] = LLC_I;
                    sharers.port1[0][llc_addr] = 0;
                    dirty_bits.port1[0][llc_addr] = 0;
                    owners.port1[0][llc_addr] = 0;
                }
                else
                {
                    wait();
                }
            }
        }
        else if (is_rst_to_get)
        {

            if (!rst_in)
            {
                /// reset
                HLS_DEFINE_PROTOCOL("is_reset");
                this->reset_state();
            }
            else
            {
                /// flush
                flush_stall = true;
                rst_flush_stalled_set = 0;
            }
        }
        else if (is_rsp_to_get)
        {

            /// @TODO handle responses
            addr_breakdown_llc_t addr_br_real;
            bitset<LLC_REQS_BITS> reqs_hit_i;
            addr_br_real.breakdown(rsp_in.addr << OFFSET_BITS);
            reqs_lookup(addr_br_real, reqs_hit_i);
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

            addr_breakdown_llc_t addr_br_real;
            bitset<LLC_REQS_BITS> reqs_hit_i, reqs_empty_i;
            addr_breakdown_llc_t evict_addr_br;
            evict_addr_br.breakdown(addr_evict_real);
            addr_br_real.breakdown(req_in.addr << OFFSET_BITS);
            set_conflict = reqs_peek_req(evict_addr_br, reqs_hit_i) || reqs_peek_req(addr_br_real, reqs_empty_i) || (reqs_cnt == 0);
            reqs_lookup(addr_br_real, reqs_hit_i);
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
                            fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_OV, hprots_buf[way], 0, req_in.line, owners_buf[way], reqs_empty_i); /// save this request in reqs buffer
                            reqs[reqs_empty_i].is_amo = true;
                        }
                    }
                    break;
                    case LLC_S:
                    {
                        int cnt = send_inv_with_sharer_list(req_in.addr, sharers_buf[way]);
                        fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_SV, req_in.hprot, 0, req_in.line, req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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
                                    fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_OS, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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
                                fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_SO, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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
                            fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_SV, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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
                            fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_SV, req_in.hprot, 0, lines_buf[way], req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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
                                fill_reqs(req_in.coh_msg, req_in.req_id, addr_br_real, 0, way, LLC_SO, req_in.hprot, 0, temp, req_in.word_mask, reqs_empty_i); /// save this request in reqs buffer
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

        else if (is_dma_req_to_get || is_dma_read_to_resume || is_dma_write_to_resume)
        {

            bool evict_dirty = 0;

            if (is_dma_req_to_get)
            {
                DMA_BURST;

                if (dma_req_in.coh_msg == REQ_DMA_READ_BURST)
                {
                    dma_read_pending = true;
                    is_dma_read_to_resume = true;
                }
                else
                {
                    dma_write_pending = true;
                    is_dma_write_to_resume = true;
                }

                dma_read_length = dma_req_in.line.range(BITS_PER_LINE - 1, BITS_PER_LINE - ADDR_BITS).to_uint();
                dma_length = 0;
                dma_done = 0;
                dma_start = true;
            }

            addr_breakdown_llc_t evict_addr_br;
            evict_addr_br.breakdown(addr_evict_real);
            bitset<LLC_REQS_BITS> reqs_empty_i;


            if (reqs_cnt == 0 || reqs_peek_req(evict_addr_br, reqs_empty_i))
            {
                /// nothing
            }
            else
            {
                if (!recall_valid && !recall_pending)
                {

                    /// Recall (may or may not evict depending on miss/hit)
                    if (state_buf[way] == LLC_V && owners_buf[way] != 0)
                    {
                        send_fwd_with_owner_mask(FWD_RVK_O, addr_evict, dma_req_in.req_id, owners_buf[way], 0);
                        fill_reqs(req_in.coh_msg, req_in.req_id, evict_addr_br, 0, way, LLC_OV, hprots_buf[way], 0, req_in.line, owners_buf[way], reqs_empty_i);
                        recall_addr = addr_evict;
                        recall_pending = true;
                    }
                    else if (state_buf[way] == LLC_S && dma_req_in.coh_msg == REQ_DMA_WRITE_BURST)
                    {
                        int cnt = send_inv_with_sharer_list(addr_evict, sharers_buf[way]);
                        fill_reqs(req_in.coh_msg, req_in.req_id, evict_addr_br, 0, way, LLC_SV, hprots_buf[way], 0, req_in.line, owners_buf[way], reqs_empty_i);
                        reqs[reqs_empty_i].invack_cnt = cnt;
                        recall_addr = addr_evict;
                        recall_pending = true;
                    }
                }

                if (!recall_pending || recall_valid)
                {

                    if (dirty_bits_buf[way])
                        evict_dirty = true;

                    /// if (evict || recall_valid) {
                    ///         owners_buf[way] = 0;
                    ///         sharers_buf[way] = 0;
                    /// }

                    if (evict)
                    {
                        /// Eviction

                        LLC_EVICT;

                        if (way == evict_ways_buf)
                        {
                            update_evict_ways = true;
                            evict_ways_buf++;
                        }

                        if (evict_dirty)
                        {
                            HLS_DEFINE_PROTOCOL("send_mem_req-6");
                            send_mem_req(WRITE, addr_evict, hprots_buf[way], lines_buf[way]);
                        }

                        state_buf[way] = LLC_I;
                    }
                    else if (recall_valid)
                    {
                        /// state_buf[way] = LLC_V;
                    }

                    /// Recall complete
                    recall_pending = 0;
                    recall_valid = 0;

                    if (is_dma_read_to_resume)
                    {
                        LLC_DMA_READ_BURST;

                        dma_length_t valid_words;
                        word_offset_t dma_read_woffset;
                        invack_cnt_t dma_info;

                        if (dma_start)
                            dma_read_woffset = dma_req_in.word_offset;
                        else
                            dma_read_woffset = 0;

                        dma_length += WORDS_PER_LINE - dma_read_woffset;

                        if (dma_length >= dma_read_length)
                            dma_done = true;

                        if (dma_start & dma_done)
                            valid_words = dma_read_length;
                        else if (dma_start)
                            valid_words = dma_length;
                        else if (dma_length > dma_read_length)
                            valid_words = WORDS_PER_LINE - (dma_length - dma_read_length);
                        else
                            valid_words = WORDS_PER_LINE;

                        if (state_buf[way] == LLC_I)
                        {

                            DMA_READ_I;
                            HLS_DEFINE_PROTOCOL("send_mem_req-7");
                            send_mem_req(READ, dma_addr, dma_req_in.hprot, 0);
                            get_mem_rsp(lines_buf[way]);
                        }

                        dma_info[0] = dma_done;
                        dma_info.range(WORD_BITS, 1) = (valid_words - 1);

                        {
                            HLS_DEFINE_PROTOCOL("send_dma_rsp_out");
                            send_dma_rsp_out(RSP_DATA_DMA, dma_addr, lines_buf[way],
                                             dma_req_in.req_id, 0, dma_info, dma_read_woffset);
                        }

                        if (state_buf[way] == LLC_I)
                        {
                            hprots_buf[way] = DATA;
                            tag_buf[way] = line_br.tag;
                            state_buf[way] = LLC_V;
                            dirty_bits_buf[way] = 0;
                        }
                    }
                    else
                    { /// is_dma_write_to_resume
                        LLC_DMA_WRITE_BURST;

                        word_offset_t dma_write_woffset = dma_req_in.word_offset;
                        dma_length_t valid_words = dma_req_in.valid_words + 1;
                        bool misaligned;

                        misaligned = (dma_write_woffset != 0 || valid_words != WORDS_PER_LINE);

                        if (state_buf[way] == LLC_I)
                        {

                            DMA_WRITE_I;

                            if (misaligned)
                            {
                                HLS_DEFINE_PROTOCOL("send_mem_req-8");
                                send_mem_req(READ, dma_addr, dma_req_in.hprot, 0);
                                get_mem_rsp(lines_buf[way]);
                            }
                        }

                        if (misaligned)
                        {
                            int word_cnt = 0;

                            for (int i = 0; i < WORDS_PER_LINE; i++)
                            {

                                HLS_UNROLL_LOOP(ON, "misaligned-dma-start-unroll");

                                if (word_cnt < valid_words && i >= dma_write_woffset)
                                {
                                    write_word(lines_buf[way], read_word(dma_req_in.line, i), i, 0, WORD);
                                    word_cnt++;
                                }
                            }
                        }
                        else
                        {

                            lines_buf[way] = dma_req_in.line;
                        }

                        lines_buf[way] = lines_buf[way];
                        dirty_bits_buf[way] = 1;

                        if (state_buf[way] == LLC_I)
                        {
                            state_buf[way] = LLC_V;
                            hprots_buf[way] = DATA;
                            tag_buf[way] = line_br.tag;
                        }

                        if (dma_req_in.hprot)
                        {

                            dma_done = true;
                        }
                    }

#ifdef LLC_DEBUG
                    dbg_length.write(dma_read_length);
                    dbg_dma_length.write(dma_length);
                    dbg_dma_done.write(dma_done);
                    dbg_dma_addr.write(dma_addr);
#endif
                    dma_addr++;
                    dma_start = 0;

                    if (dma_done)
                    {
                        dma_read_pending = 0;
                        dma_write_pending = 0;
                    }
                }
            }
        }

        /// Complete reset/flush
        if (is_flush_to_resume || is_rst_to_resume)
        {
            if (!flush_stall && !rst_stall)
            {
                HLS_DEFINE_PROTOCOL("rst_flush_done");
                do
                {
                    wait();
                } while (!llc_rst_tb_done.nb_can_put());
                llc_rst_tb_done.nb_put(1);
            }
        }

        /// -----------------------------
        /// Update cache
        /// -----------------------------

        /// update memory
        /// Resume reset
        if (is_rst_to_resume)
        {

            evict_ways.port1[0][set] = 0;

            for (int way = 0; way < LLC_WAYS; way++)
            { /// do not unroll
                const int idx = base + way;

                states.port1[0][idx] = LLC_I;
                dirty_bits.port1[0][idx] = 0;
                sharers.port1[0][idx] = 0;
            }
        }
        else if (is_rsp_to_get || is_req_to_get || is_dma_req_to_get || is_dma_read_to_resume || is_dma_write_to_resume)
        {

            tags.port1[0][llc_addr] = tag_buf[way];
            states.port1[0][llc_addr] = state_buf[way];
            lines.port1[0][llc_addr] = lines_buf[way];
            hprots.port1[0][llc_addr] = hprots_buf[way];
            owners.port1[0][llc_addr] = owners_buf[way];
            sharers.port1[0][llc_addr] = sharers_buf[way];
            dirty_bits.port1[0][llc_addr] = dirty_bits_buf[way];
            fcs_prio.port1[0][llc_addr] = fcs_prio_buf[way];

            if (update_evict_ways)
                evict_ways.port1[0][set] = evict_ways_buf;

            /// Uncomment for additional debug info during behavioral simulation
            /// const line_addr_t new_addr_evict = (tag_buf[way] << LLC_SET_BITS) + set;
            /// std::cout << std::hex << "*** way: " << way << " set: " <<  set << " addr: " << new_addr_evict << " state: " << states[llc_addr] << " line: " << lines[llc_addr] << std::endl;
        }

        {
            HLS_DEFINE_PROTOCOL("end-of-loop-break-rw-protocol");
            wait();
        }
    }
};