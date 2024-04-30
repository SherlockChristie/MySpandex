// #include "headers.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "bit_utils.hpp"
#include "msg_utils.hpp"
// 分开写函数的作用在于不用重复breakdown和fetch_line;

extern std::vector<MSG> bus;

// void LLC::msg_init()
// {
//     for (int i = 0; i < MAX_DEVS; i++)
//     {
//         // if(i==SPX) exit;
//         // else
//         // do not need, since SPX still needs RSP_NULL/REQ_NULL to indicate that this is not used.

//         // rsp init;
//         rsp_buf[i].msg = RSP_NULL;
//         LineCopy(rsp_buf[i].data, llc_data.data_line);

//         // req init;
//         rsp_buf[i].msg = REQ_NULL;
//         rsp_buf[i].gran = GRAN_WORD;
//         // rsp_buf[i].addr = REQ.addr;
//     }
// }

void LLC::breakdown(addr_t addr)
{
    llc_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    llc_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    llc_addr.index = BitSub<ADDR_SIZE, LLC_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    llc_addr.tag = BitSub<ADDR_SIZE, LLC_TAG_BITS>(addr, LLC_INDEX_BITS + WORDS_OFF + BYTES_OFF);
}

bool LLC::fetch_line()
{
    line_t zero = {0};
    unsigned long llc_index = (llc_addr.index).to_ulong();
    llc_line.line_state = line_state_buf[llc_index];
    llc_line.word_state = word_state_buf[llc_index];
    llc_line.sharers = sharers_buf[llc_index];
    if ((tag_buf[llc_index] != llc_addr.tag)) //|| (llc_line.line_state == LLC_I))
    {
        LineCopy(llc_line.data, zero);
        return 0;
        // line_t *p = (line_t *)llc_line.data_line;
        // LineCopy(*p,temp);
        // for (int i = 0; i < WORDS_PER_LINE; i++)
        // {
        //     for (int j = 0; j < BYTES_PER_WORD; j++)
        //     {
        //         llc_line.data_line[i][j] = 0;
        //     }
        // }
    }
    else
    {
        LineCopy(llc_line.data, cache[llc_index]);
        return 1;
        // for (int i = 0; i < WORDS_PER_LINE; i++)
        // {
        //     for (int j = 0; j < BYTES_PER_WORD; j++)
        //     {
        //         llc_line.data_line[i][j] = cache[llc_index][i * BYTES_PER_WORD + j];
        //     }
        // }
    }
}

// 假设是原子的，fetch_line 与 back_line 之间不会有其他操作来修改此内存行;
void LLC::back_line(DATA_LINE &llc_data)
{
    unsigned long llc_index = (llc_addr.index).to_ulong();
    LineCopy(cache[llc_index], llc_data.data);
    line_state_buf[llc_index] = llc_data.line_state;
    word_state_buf[llc_index] = llc_data.word_state;
    sharers_buf[llc_index] = llc_data.sharers;
}

// not needed
// void LLC::dev_lookup_in_llc(addr_t dev_addr)
// {
//     // Input: TU REQ addr(same with DEV addr).
//     // Goal: Find whether the data dev missed is in llc or not.
//     bool tag_hit = 0;

//     // Step 1: translate the address into llc's type.
//     SPX_ADDR llc_addr;
//     llc_addr.breakdown(dev_addr);

//     // llc_index_t index_mask = (1 << SPX_INDEX_BITS) - 1;
//     // llc_addr.index = CatBit(dev_addr->tag, dev_addr->index);
//     // llc_addr.index = ((dev_addr->tag)<<(DEV_INDEX_BITS))|(dev_addr->index);

//     // Step 2: Compare.
//     // NOTE that operate [] needs an int here, so convert bitset llc_addr.index into int first.
//     unsigned long index = (llc_addr.index).to_ulong();
//     if (tag_buf[index] == llc_addr.tag && llc_state != SPX_I)
//     {
//         tag_hit = 1;
//     }
// };

void LLC::rcv_req_single(MSG &tu_req, unsigned long offset, DATA_LINE &llc_data)
// Behaviour when LLC receives an external request from TU (Table III).
{
    MSG gen, llc_req;
    DATA_WORD data;
    WordExt(data, llc_data, offset);
    // breakdown(llc_addr, tu_req.addr);
    // fetch_line(llc_addr, llc_data);
    // msg_init();

    unsigned long req_id_int = tu_req.src.to_ulong();
    gen.id = tu_req.id;
    gen.src = tu_req.src;
    gen.dest.set(req_id_int);
    // cout<<"dest ok "<<offset<<endl;
    // Default destination: the requestor.
    // gen.mask.set(offset.to_ulong());
    gen.addr = tu_req.addr;
    // Default address: the req's addr.
    gen.gran = GRAN_WORD;
    // Default LLC granularity: word.
    gen.mask.set(offset);
    // cout<<"mask ok"<<endl;
    // Default mask.
    gen.data_line = llc_data;
    gen.data_word = data;
    // Default data.
    // msg and u_state is decided below.

    // if (!llc_data.hit)
    // {
    //     // missed, go to main memory;
    // }
    // else
    {
        switch (tu_req.msg)
        {
        case REQ_V:
        {
            // do not consider this; just miss and go to main memory;
            // but how about write ops?
            // if (data.state == SPX_I)
            // {
            //     // invalid operation for ReqV I state;
            //     gen.msg = RSP_NACK;
            // }
            if (data.state == SPX_V || data.state == SPX_S)
            {
                gen.msg = RSP_V;
            }
            else if (data.state == SPX_O)
            {
                gen.msg = FWD_REQ_V;
                gen.dest.reset(); // 清除原本的默认设置: dest = reqor;
                gen.dest.set(FindOwner(llc_data).to_ulong());
                // cout << "Really gen a fwd!!!!" << endl;
                // gen.msg_display();
            };
            break;
        }
        case REQ_S:
            // REQS2 not used;
            {
                llc_data.sharers.set(req_id_int); // Update the sharers list.
                // cout<<"sharers ok"<<endl;
                if (data.state == SPX_S) // REQS1;
                {
                    // no blocking state to S;
                    data.state = SPX_S;
                    gen.msg = RSP_S;
                }
                else if (data.state == SPX_O)
                {
                    gen.dest.reset();
                    gen.dest.set(FindOwner(llc_data).to_ulong());
                    if (gen.dest == CPU) // REQS1;
                    {
                        // go to blocking states;
                        tu_req.u_state = LLC_OS;
                        // Pay attetion: Unstable state go to the req triggers it!!!!!!!!!!!!!!!
                        gen.msg = FWD_REQ_S;
                    }
                    // 对于任何引起到O状态的转移，都应该同步更新数据域内容为所有者id!!!!!!!!!
                    else // REQS3;
                    {
                        data.data[0] = req_id_int;
                        data.state = SPX_O;
                        gen.msg = FWD_REQ_Odata;
                    }
                }
                // REQS3;
                else if (data.state == SPX_V)
                {
                    data.data[0] = req_id_int;
                    data.state = SPX_O;
                    gen.msg = RSP_S;
                }
                break;
            }
        case REQ_WT:
        {
            if (data.state == SPX_O)
            {
                gen.msg = FWD_REQ_O;
                gen.dest.reset();
                gen.dest.set(FindOwner(llc_data).to_ulong());
                data.state = SPX_V;
            }
            else if (data.state == SPX_S)
            {
                tu_req.u_state = LLC_SV; // go to blocking states;
                                         // wait();
                gen.id++;
                gen.msg = FWD_INV;
                gen.dest = InvSharers(llc_data.sharers, SPX);
            }
            else if (data.state == SPX_V || data.state == SPX_I)
            {
                gen.msg = RSP_WT;
                data.state = SPX_V;
            }
            break;
        }
        case REQ_O:
        {
            if (data.state == SPX_O)
            {
                gen.msg = FWD_REQ_O;
                gen.dest.reset();
                gen.dest.set(FindOwner(llc_data).to_ulong());
                // data.state = SPX_O;
            }

            else if (data.state == SPX_S)
            {
                tu_req.u_state = LLC_SO; // go to blocking states;
                // wait();
                gen.id++;
                gen.msg = FWD_INV;
                gen.dest = InvSharers(llc_data.sharers, SPX);
            }
            else if (data.state == SPX_V || data.state == SPX_I)
            {
                gen.msg = RSP_O;
                data.data[0] = req_id_int;
                data.state = SPX_O;
            }
            break;
        }
        case REQ_WTdata:
        {
            if (data.state == SPX_O)
            {
                gen.id++; // 如果是LLC自己的请求，应该增加id数?
                gen.msg = FWD_RVK_O;
                gen.dest.reset();
                gen.dest.set(FindOwner(llc_data).to_ulong());
                // go to blocking states;
                tu_req.u_state = LLC_OV;
            }
            else if (data.state == SPX_V)
            {
                gen.msg = RSP_WTdata;
                WordExt(data, tu_req.data_line, offset);
            }
            else if (data.state == SPX_S)
            {
                // go to blocking states;
                tu_req.u_state = LLC_SV;
                // wait();
                gen.id++;
                gen.msg = FWD_INV;
                gen.dest = InvSharers(llc_data.sharers, SPX);
            }
            break;
        }
        case REQ_Odata:
        {
            if (data.state == SPX_O)
            {
                gen.msg = FWD_REQ_Odata;
                gen.dest.reset();
                gen.dest.set(FindOwner(llc_data).to_ulong());
                // data.state == SPX_O; // no blocking states;
            }
            else if (data.state == SPX_V)
            {
                // no blocking states;
                data.data[0] = req_id_int;
                data.state = SPX_O;
                gen.msg = RSP_Odata;
            }
            else if (data.state == SPX_S)
            {
                // go to blocking states;
                tu_req.u_state = LLC_SO;
                // wait();
                gen.id++;
                gen.msg = FWD_INV;
                gen.dest = InvSharers(llc_data.sharers, SPX);
            }
            break;
        }
        case REQ_WB:
        {
            if (data.state == SPX_O)
            {
                if (tu_req.src == FindOwner(llc_data))
                {
                    data.state = SPX_V;
                    gen.msg = RSP_WB_ACK;
                }
                else
                {
                    // invalid operation for the non-owner to write;
                    gen.msg = RSP_NACK;
                }
            }
            // is that possible for a REQ_WB in SPX_S???
            else if (data.state == SPX_S)
            {
                // go to blocking states;
                tu_req.u_state = LLC_SV;
                // wait();
                gen.id++;
                gen.msg = FWD_INV;
                gen.dest = InvSharers(llc_data.sharers, SPX);
            }
            break;
        }
        }
    }

    // gen.ok_mask = ~gen.mask;
    WordIns(data, llc_data, offset);
    // back_line(llc_data);
    // std::cout << word_state_buf[0xCF] << std::endl;
    // Save changed word data state back;
    rsp_buf.push_back(gen);
    // buf_detailed(rsp_buf);
}

// void LLC::rcv_req_word(id_num_t tu_id, MSG &tu_req)
// // Behaviour when LLC receives an external request from TU (Table III).
// {
//     breakdown(tu_req.addr);
//     fetch_line();
//     // msg_init();

//     // if (tu_req.mask.any())
//     // {
//     //     WordExt(data, llc_data, tu_req.mask);
//     // }
//     // else
//     rcv_req(tu_id, tu_req, tu_req.offset, llc_line);
// }

void LLC::rcv_req_inner(MSG &tu_req, int k)
{
    breakdown(tu_req.addr);
    fetch_line();

    // if (llc_data.word_state.any())
    // // if any word in O, rsp may be different for each word;
    // {
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (tu_req.mask.test(i))
        {
            // cout<<"rcv_req ok "<< i << endl;
            rcv_req_single(tu_req, i, llc_line);
        }
        // rsp_buf.push_back(rcv_req(tu_id, tu_req, bitset<WORDS_OFF>(i), llc_data));
    }
    back_line(llc_line);

    // cout << "Before RspCoalesce!!!!!" << endl;
    // buf_detailed(rsp_buf);
    RspCoalesce(rsp_buf);
    // cout << "After RspCoalesce!!!!!" << endl;
    // buf_detailed(rsp_buf);
    bool is_rsp = 0;
    for (int i = 0; i < rsp_buf.size(); i++)
    {
        if (rsp_buf[i].id == req_buf.back().id)
        {
            req_buf.erase(req_buf.begin() + k);
            cout << "---   LLC put rsp to bus---" << endl;
            is_rsp = 1;
            break;
        }
    }
    if (!is_rsp) // 对于 rsp 和 fwd 都有的情况，req_buf也已经出队了;
    {
        cout << "---   LLC put a fwd to bus---" << endl;
    }
    put_rsp(rsp_buf);
    //}
    // else // the whole line only have 1 rsp;
    // {
    //     MSG gen_line;
    //     gen_line = rcv_req(tu_id, tu_req, 0, llc_data);
    //     gen_line.gran = GRAN_LINE;
    //     llc_data.line_state =
    // }
}

// NOTE: Word granularity doesn;t mean that it only have req for one word in a line;
// It may be a single multi-word request with a bitmask!!!!!!!!!!!!!!!!!!!!!
void LLC::rcv_req()
// LLC is always word granularity; if receive a line granularity request, breakdown into word granularity;
{
    int old_unstable = 0;
    int len = req_buf.size() - 1;
    // cout << "REQ BUF LEN A: " << len + 1 << endl;
    for (int k = len; k >= 0; k--)
    {
        // MSG tu_req = req_buf[k];
        // 不能这样。 这样相当于拷贝了一个 req_buf[k] 的副本，没能更改 req_buf[k] 的 u_state;
        // 使用引用，tu_req 成为 req_buf[k] 的别名;
        MSG &tu_req = req_buf[k];
        if (tu_req.u_state.any())
        {
            old_unstable++;
        }
        else // u_state = 0000;
        {
            rcv_req_inner(tu_req, k);
        }
    }
    // 先从后往前处理新入队的，如果新入队的操作使得阻塞状态变为非阻塞状态了，再次处理阻塞状态的req;
    // cout << "Old_unstable count: " << old_unstable << endl;
    for (int i = 0; i < old_unstable; i++)
    {
        rcv_req_inner(req_buf[i], i);
    }
}

// rcv_rsp_single can be used in any dev, go to bit_utils.hpp;

void LLC::rcv_rsp(MSG &rsp_in)
{
    breakdown(rsp_in.addr);
    fetch_line();

    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (rsp_in.mask.test(i))
        {
            rcv_rsp_single(rsp_in, i, llc_line);
        }
        // rsp_buf.push_back(rcv_req(tu_id, tu_req, bitset<WORDS_OFF>(i), llc_data));
    }
}

void LLC::solve_pending_ReqWB(id_num_t tu_id)
{
    MSG gen = req_buf.front();
    gen.msg = RSP_NACK; // 撤销此请求，不再是所有者;
    unsigned long req_id_int = tu_id.to_ulong();
    gen.dest.set(req_id_int);

    req_buf.erase(req_buf.begin()); // pop out llc's ReqWB;
    bus.push_back(gen);             // bus to pop out tu's ReqWB;
}

// void LLC::solve_pending_ReqWB(int id)
// {
//     for (int i = 0; i < req_buf.size(); i++)
//     {
//         if (req_buf[i].id == id)
//         {
//             if (req_buf[i].msg == REQ_WB)
//             {
//             }
//         }
//     }
// }