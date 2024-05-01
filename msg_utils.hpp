#ifndef MSG_UTILS_HPP
#define MSG_UTILS_HPP

#include "headers.hpp"
#include "bit_utils.hpp"
#include "blocks.hpp"
#include "consts.hpp"
#include "classes.hpp"

// void MsgInit()
// {
// }

int find_conflict(std::vector<MSG> &req_buf);
void rcv_rsp_single(MSG &rsp_in, unsigned long offset, DATA_LINE &data_line);
void get_rsp(MSG rsp, std::vector<MSG> &req_buf, id_num_t id);
void get_msg();
// void put_req(std::vector<MSG> &req)
// // Put all the req_buf to the bus.
// {
//     int req_len = req.size();
//     for (int i = 0; i < req_len; i++)
//     // while (!req.empty())
//     {
//         MSG tmp;
//         tmp = req.front();
//         bus.push_back(tmp);
//         // all req stays at the vector until its rsp in;
//     }
// }
// void put_rsp(int id);
void put_rsp(std::vector<MSG> &rsp);
// void get_req(std::vector<MSG> &req)
// // get a req, insert it in req_buf;
// {
//     req.push_back(bus.front()); // req_buf入队;
//     bus.erase(bus.begin());     // bus出队;
// }

// bool is_single(MSG &msg, DATA_LINE &line)
// {
//     if (msg.gran == GRAN_LINE)
//         return 1;
//     else
//     {
//         if (msg.mask.all())
//         {
//             if (line.word_state.none() || line.word_state.all())
//                 return 1;
//         }
//     }
//     return 0;
// }

void MsgCoalesce(std::vector<MSG> &buf, DATA_LINE &new_data);
void buf_brief(std::vector<MSG> &buf);
void buf_display();
void buf_detailed(std::vector<MSG> &buf);

#endif // MSG_UTILS_HPP