// TODO: TU 的 rcv_rsp 应该也有状态转移至 DEV !!!!!!!!!!!!!!

// 在上升沿时执行所有msg的入队操作，在下降沿时执行所有msg的出队操作;
// 如果在下一个上升沿时到来时，req队列仍然不为空，证明此时数据处于阻塞状态，后来的req加入队列等待;

// 1. Generate the message: rsp/req/fwd;
// 2. Write msg to its own fifo;
// 3. Send msg to the client's fifo;
// 4. the receiver generates message in its own fifo.

// It's a fuxking bus-based system, I should send msg to the BUS first instead of point-to-point.

// using namespace std;
#include "msg_utils.hpp"

// 将下面几个设置为工程全局变量;
extern std::vector<MSG> bus;
extern DEV devs[MAX_DEVS];
extern TU tus[MAX_DEVS];
extern LLC llc;

// void MsgInit()
// {
// }

int find_conflict(std::vector<MSG> &req_buf)
{
    int len = req_buf.size() - 1; // 不包括新入队的req自身;
    MSG new_msg = req_buf.back();
    if (!req_buf.empty())
    {
        for (int i = 0; i < len; i++)
        {
            if (new_msg.addr == req_buf[i].addr)
                return i;
        }
    }
    return 0;
}

// bool is_unstable_ok(MSG req)
// {

//     return 0;
// }

void rcv_rsp_single(MSG &rsp_in, unsigned long offset, DATA_LINE &data_line)
{
    DATA_WORD data;
    WordExt(data, data_line, offset);

    switch (rsp_in.msg)
    {
    case RSP_V:
    {
        data.state = SPX_V;
        break;
    }
    case RSP_S:
    {
        data.state = SPX_S;
        break;
    }
    case RSP_WTdata:
    {
        data.state = SPX_I;
        break;
    }
    case RSP_Odata:
    {
        data.state = SPX_O;
        break;
    }
    default:
    {
        // std::cout << "Rsp received. No state transition needed." << std::endl;
        break;
    }
    }

    WordIns(data, data_line, offset);
}

void rcv_rsp_inner(MSG &rsp_in, DATA_LINE &data_line)
{
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (rsp_in.mask.test(i))
        {
            rcv_rsp_single(rsp_in, i, data_line);
        }
    }
}

void get_rsp(MSG &rsp, std::vector<MSG> &req_buf, id_num_t id, bool is_tu)
// get a rsp, find match in req_buf;
{
    MSG gen = rsp;
    // int req_len = req_buf.size();
    for (int i = 0; i < req_buf.size(); i++)
    {
        MSG tmp = req_buf[i];
        if (rsp.id == tmp.id)
        {
            if (rsp.gran == GRAN_LINE)
            {
                // 同时释放req和rsp;
                // 但是rsp会在总线释放？
                if (tmp.time_hm < rsp.time_hm)
                    tmp.time_hm = rsp.time_hm;
                if (tmp.time_sp < rsp.time_sp)
                    tmp.time_sp = rsp.time_sp;
                if (is_tu || id == SPX)
                {
                    cout << msg_which(tmp.msg) << " done." << endl;
                    cout << "Time of H-MESI is: " << tmp.time_hm << endl;
                    cout << "Time of Spandex is: " << tmp.time_sp << endl;
                    if (id == SPX)
                    {
                        if (llc.wait_hm < tmp.time_hm)
                            llc.wait_hm = tmp.time_hm;
                        if (llc.wait_sp < tmp.time_sp)
                            llc.wait_sp = tmp.time_sp;
                    }
                }
                req_buf.erase(req_buf.begin() + i);
                // TU 的 req_buf 出队的同时 DEV 的 req_buf 也应该出队;
                // 已解决: 对于 DEV 的 req_buf 会多调用一次，但是应该不会有永不结束的递归问题;
                if (is_tu)
                    get_rsp(gen, devs[id.to_ulong()].req_buf, id, 0);
            }
            else
            {
                tmp.ok_mask |= rsp.mask;
                if (tmp.time_hm < rsp.time_hm)
                    tmp.time_hm = rsp.time_hm;
                if (tmp.time_sp < rsp.time_sp)
                    tmp.time_sp = rsp.time_sp;
                if (tmp.ok_mask.all()) // 收集到了所有rsp;
                {
                    gen.gran = GRAN_LINE;
                    cout << msg_which(tmp.msg) << " done." << endl;
                    cout << "Time of H-MESI is: " << tmp.time_hm << endl;
                    cout << "Time of Spandex is: " << tmp.time_sp << endl;
                    if (id == SPX)
                    {
                        if (llc.wait_hm < tmp.time_hm)
                            llc.wait_hm = tmp.time_hm;
                        if (llc.wait_sp < tmp.time_sp)
                            llc.wait_sp = tmp.time_sp;
                    }
                    req_buf.erase(req_buf.begin() + i);
                    if (is_tu)
                        get_rsp(gen, devs[id.to_ulong()].req_buf, id, 0);
                    // 无需释放gen, 因为 DEV 和 TU 的通信不经过总线;
                }
            }
        }
    }
}

void get_msg()
{
    while (!bus.empty())
    {
        MSG tmp = bus.front();
        // tmp.msg_display();
        if (tmp.msg < RSP_S) // req or fwd
        {
            if (tmp.dest.test(0))
            {
                // cout<<"Before req_buf push"<<endl;
                // buf_detailed(llc.req_buf);
                llc.req_buf.push_back(tmp);
                // cout<<"After req_buf push"<<endl;
                // buf_detailed(llc.req_buf);
                std::cout << "---   LLC get a req from bus---" << std::endl;
            }
            if (tmp.dest.test(1))
            {
                tus[CPU].req_buf.push_back(tmp);
                std::cout << "---TU_CPU get a req from bus---" << std::endl;
            }
            if (tmp.dest.test(2))
            {
                tus[GPU].req_buf.push_back(tmp);
                std::cout << "---TU_GPU get a req from bus---" << std::endl;
            }
            if (tmp.dest.test(3))
            {
                tus[ACC].req_buf.push_back(tmp);
                std::cout << "---TU_ACC get a req from bus---" << std::endl;
            }
        }
        else // rsp;
        {
            // if (tmp.dest.test(0))
            //     llc.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(1))
            //     tcpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(2))
            //     tgpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(3))
            //     tacc.rsp_buf.push_back(tmp);

            // 无需将rsp_in入队，直接对对碰消掉req和rsp_in;
            if (tmp.dest.test(0))
            {
                std::cout << "---   LLC get a rsp from bus---" << std::endl;
                llc.rcv_rsp(tmp);
            }
            if (tmp.dest.test(1))
            {
                std::cout << "---TU_CPU get a rsp from bus---" << std::endl;
                tus[CPU].rcv_rsp(tmp);
            }
            if (tmp.dest.test(2))
            {
                // tmp.time_hm += TIME_L2_AVG;
                std::cout << "---TU_GPU get a rsp from bus---" << std::endl;
                tus[GPU].rcv_rsp(tmp);
            }
            if (tmp.dest.test(3))
            {
                std::cout << "---TU_ACC get a rsp from bus---" << std::endl;
                tus[ACC].rcv_rsp(tmp);
            }
        }
        bus.erase(bus.begin());
        // buf_detailed(bus);
    }
    buf_display();
}

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
// void put_rsp_inner(std::vector<MSG> &rsp)
void put_rsp(std::vector<MSG> &rsp)
// Put all the rsp_buf to the bus.
{
    int rsp_len = rsp.size();
    for (int i = 0; i < rsp_len; i++)
    // while (!rsp.empty())
    {
        MSG tmp;
        tmp = rsp.front();
        tmp.msg_display();
        bus.push_back(tmp);
        // if (tmp.msg < FWD_REQ_S) // rsp, not fwd;
        // {
        rsp.erase(rsp.begin());
        //}
    }
    buf_display();
}

// void put_rsp(int id)
// {
//     switch (id)
//     {
//     case SPX:
//         put_rsp_inner(llc.rsp_buf);
//         std::cout << "LLC put rsp to bus---" << std::endl;
//         break;
//     case CPU:
//         put_rsp_inner(tus[CPU].rsp_buf);
//         std::cout << "TU_CPU put rsp to bus---" << std::endl;
//         break;
//     case GPU:
//         put_rsp_inner(tus[GPU].rsp_buf);
//         std::cout << "TU_GPU put rsp to bus---" << std::endl;
//         break;
//     case ACC:
//         put_rsp_inner(tus[ACC].rsp_buf);
//         std::cout << "TU_ACC put rsp to bus---" << std::endl;
//         break;
//     default:
//         break;
//     }
// }

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

void MsgCoalesce(std::vector<MSG> &buf, DATA_LINE &new_data)
{
    // int len = buf.size();
    // 不应该用固定长度的len!!!!!!!!!!!!!!!!!!!!
    for (int i = 0; i < buf.size() - 1; i++)
    {
        for (int j = i + 1; j < buf.size(); j++)
        {
            if ((buf[i].id == buf[j].id) && (buf[i].dest == buf[j].dest) && (buf[i].addr == buf[j].addr) && (buf[i].msg == buf[j].msg))
            {
                buf[i].mask |= buf[j].mask;
                // WordIns(buf[j].data_word,buf[i].data_line,buf[j].mask.to_ulong());
                // // buf[j].mask 应当只有一位，不会出错;
                if (buf[i].time_hm < buf[j].time_hm)
                    buf[i].time_hm = buf[j].time_hm;
                if (buf[i].time_sp < buf[j].time_sp)
                    buf[i].time_sp = buf[j].time_sp;
                // buf[i].time_hm = max(buf[i].time_hm, buf[j].time_hm);
                buf.erase(buf.begin() + j);
            }
        }
    }
    for (int i = 0; i < buf.size(); i++)
    {
        buf[i].ok_mask = ~buf[i].mask;
        buf[i].data_line = new_data;
    }
}

void buf_brief(std::vector<MSG> &buf)
{
    int len = buf.size();
    for (int i = 0; i < len; i++)
    // for (int i = len-1; i >= 0; i--)
    {
        std::cout << buf[i].id << " " << msg_which(buf[i].msg) << " ";
    }
    std::cout << std::endl;
}

void buf_display()
{
    std::cout << "---------------------BUF_DISPLAY---------------------" << std::endl;
    // std::cout << "--BUS--|--LLC--|--CPU--|--ACC--|--GPU--" << std::endl;
    std::cout << "--BUS--" << std::endl;
    buf_brief(bus);
    std::cout << "--LLC--" << std::endl;
    buf_brief(llc.req_buf);
    std::cout << "--CPU--" << std::endl;
    buf_brief(tus[CPU].req_buf);
    std::cout << "--GPU--" << std::endl;
    buf_brief(tus[GPU].req_buf);
    std::cout << "--ACC--" << std::endl;
    buf_brief(tus[ACC].req_buf);
    std::cout << "-----------------------------------------------------" << std::endl;
}

void buf_detailed(std::vector<MSG> &buf)
{
    std::cout << "--------------------BUF_DETAILED_INFO-------------------" << std::endl;
    int len = buf.size();
    if (!len)
    {
        std::cout << "Nothing in the buffer!" << std::endl;
    }
    for (int i = 0; i < len; i++)
    {
        buf[i].msg_display();
    }
    std::cout << "--------------------------------------------------------" << std::endl;
}