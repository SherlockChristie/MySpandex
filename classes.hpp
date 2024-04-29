#ifndef __CLASSES_HPP__
#define __CLASSES_HPP__
// TODO: 将部分属性改为private; 用继承优化类的实现;
#include "headers.hpp"
#include "consts.hpp"
#include "blocks.hpp"
using namespace std;

// Why not just using #include <vector> instead of creating a FIFO by yourself...
// class FIFO {
// public:
//     MSG q[MAX_MSG];
//     int front;
//     int rear;
//     int count;

// public:
//     FIFO() : front(0), rear(0), count(0) {}

//     bool isEmpty() const {
//         return count == 0;
//     }

//     bool isFull() const {
//         return count == MAX_MSG;
//     }

//     void envector(MSG element) {
//         if (isFull()) {
//             // throw std::overflow_error("Queue is full");
//         }

//         q[rear] = element;
//         rear = (rear + 1) % MAX_MSG;
//         ++count;
//     }

//     MSG devector() {
//         if (isEmpty()) {
//             // throw std::underflow_error("Queue is empty");
//         }

//         MSG element = q[front];
//         front = (front + 1) % MAX_MSG;
//         --count;
//         return element;
//     }

//     MSG peek() const {
//         if (isEmpty()) {
//             // throw std::underflow_error("Queue is empty");
//         }
//         return q[front];
//     }

//     // void display() const {
//     //     std::cout << "Queue: ";
//     //     for (int i = 0; i < count; ++i) {
//     //         std::cout << q[(front + i) % MAX_MSG] << " ";
//     //     }
//     //     std::cout << std::endl;
//     // }
// };

class DEV
{
public:
    id_num_t dev_id; // 设备类型;
    std::vector<MSG> req_buf;
    // Wrong comprehension, dismiss the message below.
    // REQ req[MAX_DEVS];
    // // 同一时间内一个设备最多向MAX_DEVS-1(自己)个目标发送请求;
    // // 尽管如此，数组大小也仍应该是MAX_DEVS而非MAX_DEVS-1，因为req[id]中的id对应谁是固定的;
    // // 数组下标代表引起此req的src，req.dest代表此req的目标（总是假定不会同时发送）;
    std::vector<MSG> rsp_buf;
    DEV_ADDR dev_addr;
    DATA_LINE dev_line;
    DATA_WORD dev_word;
    byte_t cache[DEV_ROW][DEV_COL];
    // 内存是按字节寻址的，故此处用byte_t; 但数据传输的最小单元是字，故下文用word_t;
    // word_t data_word;
    // line_t data_line;
    state_t state_buf[DEV_ROW];
    dev_tag_t tag_buf[DEV_ROW];
    id_bit_t sharers_buf[DEV_ROW];

    // void msg_init();
    void breakdown(addr_t addr);
    bool fetch_line();
    void back_line(DATA_LINE &tu_data);
    // void send_rsp(int msg, int REQ_id, bool to_REQ, addr_t line_addr, line_t &line);
    // void rcv_fwd(MSG &fwd_in);
    // void dev_caller_tu();
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
    id_num_t tu_id;
    std::vector<MSG> req_buf;
    std::vector<MSG> rsp_buf;
    DATA_LINE tu_line;
    DATA_WORD tu_word;

    // void msg_init();
    void tst();
    void req_mapping(unsigned long id, MSG &dev_req);
    void state_mapping(unsigned long id, DATA_LINE &data_line, DATA_WORD &data_word);
    void mapping_wrapper(DEV &dev);
    // void rcv_fwd(id_num_t &reqor_id, MSG &fwd_in, word_offset_t offset, DATA_LINE &dev_line);
    // void rcv_fwd_word(id_num_t &reqor_id, DEV &owner_dev, MSG &fwd_in);
    // void rcv_fwd_line(id_num_t &reqor_id, DEV &owner_dev, MSG &fwd_in);
    void rcv_fwd_single(MSG &fwd_in, unsigned long offset);
    void rcv_fwd();
    // void tu_for_gpu();
    // void tu_for_acc();
    // void tu_for_cpu(TU &sender);
    // void tu_callee_dev();
};

class LLC
{
public:
    std::vector<MSG> req_buf;
    std::vector<MSG> rsp_buf;
    LLC_ADDR llc_addr;
    DATA_LINE llc_line;
    DATA_WORD llc_word;
    // word_t data_word;
    // line_t data_line;
    byte_t cache[LLC_ROW][LLC_COL];
    state_t line_state_buf[LLC_ROW];
    spx_word_state_t word_state_buf[LLC_ROW];
    llc_tag_t tag_buf[LLC_ROW];
    id_bit_t sharers_buf[LLC_ROW];

    // void msg_init();
    void breakdown(addr_t addr);
    bool fetch_line();
    void back_line(DATA_LINE &llc_data);
    // id_bit_t FindOwner(DATA_LINE &llc_data);
    // void rcv_req(id_num_t tu_id, MSG &tu_req, word_offset_t offset, DATA_LINE &llc_data);
    // void rcv_req_word(id_num_t tu_id, MSG &tu_req);
    // void rcv_req_line(id_num_t tu_id, MSG &tu_req);
    void rcv_req_single(MSG &tu_req, unsigned long offset, DATA_LINE &llc_data);
    void rcv_req();
    void rcv_rsp(MSG &rsp_in);
    // void rcv_rsp_line(MSG &rsp_in);
    // void snd_req();
    // void snd_rsp();
    void solve_pending_ReqWB(id_num_t tu_id);
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
//     id_bit_t sharers;
//     void fetch_line(LLC &llc, LLC_ADDR &llc_addr);
// };

// class MEM
// {
// public:
//     int REQ;
//     int RSP;
//     word_t data_word;
//     line_t data_line;
//     byte_t cache[MEM_ROW][MEM_COL];
//     bool state_buf[MEM_ROW][STATE_BITS];
//     void llc_lookup_in_mem(addr_t *addr, bool &evict);
// };

// std::vector<MSG> bus;
// // DEV cpu, gpu, acc;
// // TU tcpu, tgpu, tacc;
// DEV devs[MAX_DEVS];
// TU tus[MAX_DEVS];
// LLC llc;

#endif // __CLASSES_HPP__