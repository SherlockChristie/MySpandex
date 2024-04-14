#ifndef __CLASSES_HPP__
#define __CLASSES_HPP__

#include <cstdint> // For uint8_t, uint32_t etc.
#include <cmath>
#include "consts.hpp"
#include "blocks.hpp"
using namespace std;

class DEV
{
public:
    uint8_t type; // 设备类型;
    DEV_REQ req;
    DEV_RSP rsp;
    DEV_ADDR dev_addr;
    DEV_DATA dev_data;
    byte_t cache[DEV_ROW][DEV_COL];
    // 内存是按字节寻址的，故此处用byte_t; 但数据传输的最小单元是字，故下文用word_t;
    state_t state_buf[DEV_ROW];
    dev_tag_t tag_buf[DEV_ROW];
    sharers_t sharers_buf[DEV_ROW];
    void breakdown(DEV_ADDR &dev_addr, addr_t addr);
    void fetch_line(DEV_ADDR &dev_addr, DEV_DATA &dev_data);
    void send_rsp(uint8_t msg, uint8_t req_id, bool to_req, addr_t line_addr, line_t &line);
    void snd_rsp(LLC_REQ &fwd_in);
    void solve_pending_ReqWB();
};

// class DEV_ADDR
// {
// public:
//     dev_tag_t tag;
//     dev_index_t index;
//     word_offset_t w_off;
//     byte_offset_t b_off;
//     void breakdown(addr_t addr);
// };

class TU
{
public:
    uint8_t type;
    TU_REQ req;
    TU_RSP rsp;
    word_t data_word;
    line_t data_line;
    void req_mapping(DEV &dev);
    void state_mapping();
};

class LLC
{
public:
    LLC_REQ req;
    LLC_RSP rsp;
    LLC_ADDR llc_addr;
    LLC_DATA llc_data;
    // word_t data_word;
    // line_t data_line;
    byte_t cache[LLC_ROW][LLC_COL];
    state_t state_buf[LLC_ROW];
    llc_tag_t tag_buf[LLC_ROW];
    sharers_t sharers_buf[LLC_ROW];
    void breakdown(LLC_ADDR &llc_addr, addr_t addr);
    void fetch_line(LLC_ADDR &llc_addr, LLC_DATA &llc_data);
    void forwards(TU &owner);
    void rcv_req(TU &tu);
    void dev_lookup_in_llc(addr_t dev_addr);
    void ctrl(); // Processes.
    // void reset_io();// cache controller
    // void reset_state();
    // void read_set(llc_addr_t base, llc_way_t offset);
    // void send_mem_req(bool hwrite, line_addr_t line_addr, hprot_t hprot, line_t line);
    // void get_mem_rsp(line_t &line);
    // void send_rsp_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, cache_id_t req_id, cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t_t word_offset_t, word_mask_t word_mask);
    // void send_fwd_out(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, cache_id_t dest_id, word_mask_t word_mask);
    // void send_fwd_out_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, cache_id_t dest_id, word_mask_t word_mask, line_t data);
    // bool send_fwd_with_owner_mask(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, word_mask_t word_mask, line_t data);
    // // returns if any fwd was sent
    // bool send_fwd_with_owner_mask_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t req_id, word_mask_t word_mask, line_t data, line_t data_out);
    // int send_inv_with_sharer_list(line_addr_t addr, sharers_t sharer_list);
    // // returns number of incack to expect
    // void send_dma_rsp_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, llc_coh_dev_id_t req_id, cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t_t word_offset_t);
    // bool is_amo(coh_msg_t coh_msg);
    // void fill_reqs(mix_msg_t msg, cache_id_t req_id, addr_breakdown_llc_t addr_br, llc_tag_t tag_estall, llc_way_t way_hit, llc_unstable_state_t state, hprot_t hprot, word_t word, line_t line, word_mask_t word_mask, sc_uint<LLC_REQS_BITS> reqs_i);
    // // write reqs buf

    // bool reqs_peek_req(addr_breakdown_llc_t br, sc_uint<LLC_REQS_BITS> &reqs_empty_i);
};

// class LLC_ADDR
// {
// public:
//     llc_tag_t tag;
//     llc_index_t index;
//     word_offset_t w_off;
//     byte_offset_t b_off;
//     void breakdown(addr_t addr);
// };

// class LLC_DATA
// {
// public:
//     bool hit;
//     word_t data_line[WORDS_PER_LINE]; // word granularity;
//     state_t state;
//     sharers_t sharers;
//     void fetch_line(LLC &llc, LLC_ADDR &llc_addr);
// };

class MEM
{
public:
    uint8_t req;
    uint8_t rsp;
    word_t data_word;
    line_t data_line;
    byte_t cache[MEM_ROW][MEM_COL];
    bool state_buf[MEM_ROW][STATE_BITS];
    void llc_lookup_in_mem(addr_t *addr, bool &evict);
};

// class llc_mem_req_t
// {
// public:
//     bool op; /// r, w, r atom., w atom., flush
//     // hsize_t	hsize;
//     LLC_DATA_ADDR addr;
//     line_t data_line;
// };

#endif // __CLASSES_HPP__