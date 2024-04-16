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
    id_t id;               // 设备类型;
    REQ req_buf[MAX_MSG];
    // Wrong comprehension, dismiss the message below.
    // REQ req[MAX_DEVS];
    // // 同一时间内一个设备最多向MAX_DEVS-1(自己)个目标发送请求;
    // // 尽管如此，数组大小也仍应该是MAX_DEVS而非MAX_DEVS-1，因为req[id]中的id对应谁是固定的;
    // // 数组下标代表引起此req的src，req.dest代表此req的目标（总是假定不会同时发送）;
    RSP rsp_buf[MAX_MSG];
    DEV_ADDR dev_addr;
    DATA_LINE dev_data;
    byte_t cache[DEV_ROW][DEV_COL];
    // 内存是按字节寻址的，故此处用byte_t; 但数据传输的最小单元是字，故下文用word_t;
    // word_t data_word;
    // line_t data_line;
    dev_state_t state_buf[DEV_ROW];
    dev_tag_t tag_buf[DEV_ROW];
    sharers_t sharers_buf[DEV_ROW];

    void msg_init();
    void breakdown(DEV_ADDR &dev_addr, addr_t addr);
    bool fetch_line(DEV_ADDR &dev_addr, DATA_LINE &dev_data);
    void send_RSP(uint8_t msg, uint8_t REQ_id, bool to_REQ, addr_t line_addr, line_t &line);
    void snd_RSP(REQ &fwd_in);
    void solve_pending_REQWB();
    void dev_caller_tu();
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
    id_t id;
    REQ req_buf[MAX_MSG];
    RSP rsp_buf[MAX_MSG];
    DATA_LINE tu_data;

    void msg_init();
    void req_mapping(unsigned long id, REQ &req);
    void state_mapping(unsigned long id, DATA_LINE &dev_data);
    void mapping_wrapper(DEV &dev);
    void tu_for_gpu();
    void tu_for_acc();
    void tu_for_cpu(TU &sender);
    void tu_callee_dev();
};

class LLC
{
public:
    REQ req_buf[MAX_MSG];
    RSP rsp_buf[MAX_MSG];
    LLC_ADDR llc_addr;
    DATA_LINE llc_data;
    // word_t data_word;
    // line_t data_line;
    byte_t cache[LLC_ROW][LLC_COL];
    llc_state_t state_buf[LLC_ROW];
    llc_tag_t tag_buf[LLC_ROW];
    sharers_t sharers_buf[LLC_ROW];

    void msg_init();
    void breakdown(LLC_ADDR &llc_addr, addr_t addr);
    bool fetch_line(LLC_ADDR &llc_addr, DATA_LINE &llc_data);
    id_t find_owner(DATA_LINE &llc_data);
    void rcv_req(id_t &tu_id, REQ &tu_req);
    void snd_req();
    void snd_rsp();
    // void dev_lookup_in_llc(addr_t dev_addr);
    // void ctrl(); // Processes.
    // void reset_io();// cache controller
    // void reset_state();
    // void read_set(llc_addr_t base, llc_way_t offset);
    // void send_mem_REQ(bool hwrite, line_addr_t line_addr, hprot_t hprot, line_t line);
    // void get_mem_RSP(line_t &line);
    // void send_RSP_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, cache_id_t REQ_id, cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t_t word_offset_t, word_mask_t word_mask);
    // void send_fwd_out(mix_msg_t coh_msg, line_addr_t addr, cache_id_t REQ_id, cache_id_t dest_id, word_mask_t word_mask);
    // void send_fwd_out_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t REQ_id, cache_id_t dest_id, word_mask_t word_mask, line_t data);
    // bool send_fwd_with_owner_mask(mix_msg_t coh_msg, line_addr_t addr, cache_id_t REQ_id, word_mask_t word_mask, line_t data);
    // // returns if any fwd was sent
    // bool send_fwd_with_owner_mask_data(mix_msg_t coh_msg, line_addr_t addr, cache_id_t REQ_id, word_mask_t word_mask, line_t data, line_t data_out);
    // int send_inv_with_sharer_list(line_addr_t addr, sharers_t sharer_list);
    // // returns number of incack to expect
    // void send_dma_RSP_out(coh_msg_t coh_msg, line_addr_t addr, line_t line, llc_coh_dev_id_t REQ_id, cache_id_t dest_id, invack_cnt_t invack_cnt, word_offset_t_t word_offset_t);
    // bool is_amo(coh_msg_t coh_msg);
    // void fill_REQs(mix_msg_t msg, cache_id_t REQ_id, addr_breakdown_llc_t addr_br, llc_tag_t tag_estall, llc_way_t way_hit, llc_unstable_state_t state, hprot_t hprot, word_t word, line_t line, word_mask_t word_mask, sc_uint<REQS_BITS> REQs_i);
    // // write REQs buf

    // bool REQs_peek_REQ(addr_breakdown_llc_t br, sc_uint<REQS_BITS> &REQs_empty_i);
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

// class DATA_LINE
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
    uint8_t REQ;
    uint8_t RSP;
    word_t data_word;
    line_t data_line;
    byte_t cache[MEM_ROW][MEM_COL];
    bool state_buf[MEM_ROW][STATE_BITS];
    void llc_lookup_in_mem(addr_t *addr, bool &evict);
};

// class llc_mem_REQ_t
// {
// public:
//     bool op; /// r, w, r atom., w atom., flush
//     // hsize_t	hsize;
//     DATA_LINE_ADDR addr;
//     line_t data_line;
// };

class FIFO {
private:
    int arr[MAX_MSG];
    int front;
    int rear;
    int count;

public:
    FIFO() : front(0), rear(0), count(0) {}

    bool isEmpty() const {
        return count == 0;
    }

    bool isFull() const {
        return count == MAX_MSG;
    }

    void enqueue(int element) {
        if (isFull()) {
            throw std::overflow_error("Queue is full");
        }

        arr[rear] = element;
        rear = (rear + 1) % MAX_MSG;
        ++count;
    }

    int dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }

        int element = arr[front];
        front = (front + 1) % MAX_MSG;
        --count;
        return element;
    }

    int peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        return arr[front];
    }

    // void display() const {
    //     std::cout << "Queue: ";
    //     for (int i = 0; i < count; ++i) {
    //         std::cout << arr[(front + i) % MAX_MSG] << " ";
    //     }
    //     std::cout << std::endl;
    // }
};

#endif // __CLASSES_HPP__