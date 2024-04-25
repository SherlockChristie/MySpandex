inline void llc_spandex::reset_state()
{
    for (int i = 0; i < LLC_WAYS; i++) {
        HLS_UNROLL_LOOP(ON, "reset-bufs");

        tags_buf[i] = 0;
        states_buf[i] = 0;
        lines_buf[i] = 0;
        sharers_buf[i] = 0;
        owners_buf[i] = 0;
        dirty_bits_buf[i] = 0;
        hprots_buf[i] = 0;
        fcs_prio_buf[i] = 0;
    }
    wait();
    for (int i = 0; i < LLC_N_REQS; i++)
    {
        REQs[i].state = LLC_I;
        wait();
    }
    REQs_cnt = LLC_N_REQS;
    set_conflict = false;
    evict_stall = false;
    evict_inprogress = false;

    dma_read_pending = false;
    dma_write_pending = false;
    dma_addr = 0;
    dma_read_length = 0;
    dma_length = 0;
    dma_done = false;
    dma_start = false;
    recall_pending = false;
    recall_valid = false;
    recall_addr = 0;

    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        fwd_coal_word_mask[i] = 0;
        fwd_coal_temp_dest[i] = 0;
    }


#ifdef LLC_DEBUG
//     dbg_asserts.write(0);
//     dbg_bookmark.write(0);

    dbg_is_rst_to_get.write(0);
    dbg_is_RSP_to_get.write(0);
    dbg_is_REQ_to_get.write(0);

    dbg_tag_hit.write(0);
    dbg_hit_way.write(0);
    dbg_empty_way_found.write(0);
    dbg_empty_way.write(0);
    dbg_way.write(0);
    dbg_llc_addr.write(0);
    dbg_evict.write(0);

    dbg_length.write(0);
    dbg_dma_length.write(0);
    dbg_dma_done.write(0);
    dbg_dma_addr.write(0);
    dbg_dma_read_pending.write(0);
    dbg_dma_write_pending.write(0);

//     dbg_length.write(0);
#endif

    wait();
}

inline void llc_spandex::send_mem_REQ(bool hwrite, line_addr_t addr, hprot_t hprot, line_t line)
{
    SEND_MEM_REQ;

    llc_mem_REQ_t mem_REQ;
    mem_REQ.hwrite = hwrite;
    mem_REQ.addr = addr;
    mem_REQ.hsize = WORD;
    mem_REQ.hprot = hprot;
    mem_REQ.line = line;
    do {wait();}
    while (!llc_mem_REQ.nb_can_put());
    llc_mem_REQ.nb_put(mem_REQ);
}

inline void llc_spandex::get_mem_RSP(line_t &line)
{
    GET_MEM_RSP;

    llc_mem_RSP_t mem_RSP;
    while (!llc_mem_RSP.nb_can_get())
        wait();
    llc_mem_RSP.nb_get(mem_RSP);
    line = mem_RSP.line;
}

inline bool llc_spandex::send_fwd_with_owner_mask(mix_msg_t coh_msg, line_addr_t addr, cache_id_bit_t REQ_id, word_mask_t word_mask, line_t data)
{
    fwd_coal_send_count = 0;
    for (int i = 0; i < WORDS_PER_LINE; i++) {
        if (word_mask & (1 << i)) {
            int owner = data.range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD).to_int();
            if (owner != REQ_id || coh_msg == FWD_RVK_O) { // skip if REQuestor already owns word, still send if revoke
                bool coal = false;
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
                // new forward dest
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
        send_fwd_out(coh_msg, addr, REQ_id, fwd_coal_temp_dest[i], fwd_coal_word_mask[i]);
        fwd_coal_word_mask[i] = 0;
        wait();
    }
    return fwd_coal_send_count > 0;
}

inline bool llc_spandex::send_fwd_with_owner_mask_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_bit_t REQ_id, word_mask_t word_mask, line_t data, line_t data_out)
{
    fwd_coal_send_count = 0;
    for (int i = 0; i < WORDS_PER_LINE; i++) {
        if (word_mask & (1 << i)) {
            int owner = data.range(CACHE_ID_WIDTH - 1 + i * BITS_PER_WORD, i * BITS_PER_WORD).to_int();
            if (owner != REQ_id || coh_msg == FWD_RVK_O) { // skip if REQuestor already owns word, still send if revoke
                bool coal = false;
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
                // new forward dest
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
        send_fwd_out_data(coh_msg, addr, REQ_id, fwd_coal_temp_dest[i], fwd_coal_word_mask[i], data_out);
        fwd_coal_word_mask[i] = 0;
        wait();
    }
    return fwd_coal_send_count > 0;
}

inline int llc_spandex::send_inv_with_sharer_list(line_addr_t addr, id_bit_t sharer_list)
{
    HLS_DEFINE_PROTOCOL("inv_all_sharers");
    int cnt = 0;
    for (int i = 0; i < MAX_N_L2; i++) {
        if (sharer_list & (1 << i)) {
            send_fwd_out(FWD_INV_SPDX, addr, 0, i, WORD_MASK_ALL);
            cnt++;
        }
        wait();
    }
    return cnt;
}