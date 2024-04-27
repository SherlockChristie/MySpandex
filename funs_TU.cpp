// TODO: 修改不在预期状态的 FWD_INV;

#include "classes.hpp"
#include "bit_utils.hpp"
#include "msg_utils.hpp"
using namespace std;

// void TU::msg_init()
// {
//     for (int i = 0; i < MAX_DEVS; i++)
//     {
//         // rsp init;
//         rsp_buf[i].msg = RSP_NULL;
//         LineCopy(rsp_buf[i].data, tu_data.data_line);

//         // req init;
//         req_buf[i].msg = REQ_NULL;
//         req_buf[i].gran = GRAN_WORD;
//         // req_buf[i].addr = REQ.addr;
//     }
// }

void TU::req_mapping(unsigned long id, MSG &dev_req)
// Translate device message into LLC message (Table II).
{
    // REQ self_req = req_buf[id];
    // 不行，这样没有修改TU中req数组的值，还是得用指针;
    MSG gen = dev_req;
    // REQ *self_req = req_buf + id;
    gen.dest.set(SPX);

    switch (id)
    {
    case GPU:
    {
        if (dev_req.msg == READ)
        {
            gen.msg = REQ_V;
            gen.gran = GRAN_LINE;
        }
        else if (dev_req.msg == WRITE)
        {
            gen.msg = REQ_WT;
            gen.gran = GRAN_WORD;
        }
        else if (dev_req.msg == RMW)
        {
            gen.msg = REQ_WTdata;
            gen.gran = GRAN_WORD;
        }
        break;
    }
    case ACC:
    {
        if (dev_req.msg == READ)
        {
            gen.msg = REQ_V;
            gen.gran = GRAN_WORD;
        }
        else if (dev_req.msg == WRITE)
        {
            gen.msg = REQ_O;
            gen.gran = GRAN_WORD;
        }
        else if (dev_req.msg == RMW)
        {
            gen.msg = REQ_Odata;
            gen.gran = GRAN_WORD;
        }
        else if (dev_req.msg == O_REPL)
        {
            gen.msg = REQ_WB;
            gen.gran = GRAN_WORD;
        }
        break;
    }
    case CPU:
    {
        if (dev_req.msg == READ)
        {
            gen.msg = REQ_S;
            gen.gran = GRAN_LINE;
        }
        else if (dev_req.msg == WRITE || dev_req.msg == RMW)
        {
            gen.msg = REQ_Odata;
            gen.gran = GRAN_LINE;
        }
        else if (dev_req.msg == O_REPL)
        {
            gen.msg = REQ_WB;
            gen.gran = GRAN_LINE;
        }
        break;
    }
    }
    if (gen.gran == GRAN_LINE)
    {
        gen.mask.set();
    }
    // else gen.mask = dev_req.mask;
    gen.ok_mask = ~gen.mask;
    tus[id].req_buf.push_back(gen);
}

void TU::state_mapping(unsigned long id, DATA_LINE &data_line, DATA_WORD &data_word)
// Translate device state into LLC state (Section III-D).
{
    // bitset<STATE_LINE> state_line = BitSub<STATE_BITS, STATE_LINE>(data_line.line_state, 0);
    // bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(data_line.line_state, 0);
    switch (id)
    {
    case GPU:
    {
        if (data_word.state == DEV_I)
            tu_word.state = SPX_I;
        else if (data_word.state == DEV_V)
            tu_word.state = SPX_V;
        break;
    }
    case ACC:
    {
        if (data_line.line_state == DEV_I)
            tu_line.line_state = LLC_I;
        else if (data_line.line_state == DEV_V)
            tu_line.line_state = LLC_V;
        else if (data_line.line_state == DEV_O)
        {
            tu_line.line_state = LLC_V;
            tu_line.word_state.set();
        }
        break;
    }
    case CPU:
    {
        if (data_line.line_state == DEV_I)
            tu_line.line_state = LLC_I;
        else if (data_line.line_state == DEV_S)
            tu_line.line_state = LLC_S;
        else if (data_line.line_state == DEV_M || data_line.line_state == DEV_E)
        {
            tu_line.line_state = LLC_V;
            tu_line.word_state.set();
        }
        break;
    }
    default:
        tu_line.line_state = LLC_I;
        break;
    }
}

void TU::mapping_wrapper(DEV &dev)
{
    unsigned long id = tu_id.to_ulong();
    req_mapping(id, dev.req_buf.front());
    state_mapping(id, dev.dev_line, dev.dev_word);
    // dev.req_buf.erase(req_buf.begin());
}

// void TU::tu_for_gpu()
// // Required functionality for GPU coherence TU (Section III-D).
// // If ReqV starve, go to this function;
// {
//     id_num_t id(GPU);
//     // support REQV retries;
//     int retry_times;
//     rsp_buf[id].msg = RSP_NACK;
//     if (retry_times < MAX_RETRY)
//     {
//         req_buf[id].msg = REQ_V;
//     }
//     else
//     {
//         req_buf[id].msg = REQ_WTdata;
//     }
//     //  handle partial word granularity responses;
//     if (LineReady(state_line))
//     {
//         tu_callee_dev();
//     }
//     else
//     {
//         wait();
//     }
// }

// void TU::tu_for_acc()
// // REQuired functionality for DeNovo TU (Section III-D).
// {
//     unsigned long id = ACC;
//     // support REQV retries;
//     int retry_times;
//     rsp_buf[id].msg = RSP_NACK;
//     if (retry_times < MAX_RETRY)
//     {
//         req_buf[id].msg = REQ_V;
//     }
//     else
//     {
//         req_buf[id].msg = REQ_WTdata;
//         // or REQ_Odata?
//     }
// }

// void TU::tu_callee_dev()
// {
//     int retry_times;
//     rsp_buf[id].msg = RSP_NACK;
//     if (retry_times < MAX_RETRY)
//     {
//         req_buf[id].msg = REQ_V;
//     }
//     else
//     {
//         req_buf[id].msg = REQ_WTdata;
//     }
// }
// void TU::tu_for_cpu(MSG &fwd_in)
// {
//     MSG gen = fwd_in;
//     gen.id++;
//     gen.dest.set(SPX); 
//     gen.addr = fwd_in.addr;
//     gen.gran = GRAN_WORD;
//     gen.mask = ~fwd_in.mask;
//     gen.ok_mask = fwd_in.mask; //~gen.mask;
//     gen.msg = REQ_WB;

//     tus[CPU].req_buf.push_back(gen);
// }

void TU::rcv_fwd_single(id_num_t &reqor_id, MSG &fwd_in, unsigned long offset)
{
    // MSG fwd_in = req_buf.front(); // pushed in req_mapping();
    bool conflict_flag = is_conflict(req_buf, fwd_in);
    MSG pending;
    if (conflict_flag)
    {
        pending = req_buf.front();
    }

    req_buf.push_back(fwd_in);

    DATA_WORD data;
    WordExt(data, tu_line, offset);

    MSG gen_reqor, gen_llc;
    // Send rsp for only LLC if RvkO/Inv; otherwise send rsp fot both;

    gen_reqor.addr = fwd_in.addr;
    // Default address: the req's addr.
    gen_reqor.gran = GRAN_WORD;
    // Default LLC granularity: word.
    gen_reqor.mask.set(offset);
    // Default mask.
    gen_reqor.data_line = tu_line;
    gen_reqor.data_word = data;
    // Default data.
    // msg and u_state is decided below.

    gen_reqor.dest.set(reqor_id.to_ulong()); // go to reqor;
    gen_llc = gen_reqor;                     // Default items are the same except the dest.
    gen_llc.dest.set(SPX);                   // go to LLC;
    gen_llc.msg = RSP_FWD;                   // bus to pop out llc's FWD;

    switch (fwd_in.msg)
    {
    case FWD_REQ_V:
        // Forwards to owner; Others can read; Remain O state;
        {
            // In the unstable states;
            if (conflict_flag)
            {
                // Pending Transition to Expected State;
                if (pending.u_state == DEV_XO)
                {
                    if (pending.msg == REQ_Odata || pending.msg == FWD_REQ_Odata)
                    {
                        // already push in;
                        // just wait;
                    }
                    else
                    {
                        gen_reqor.msg = RSP_V;
                        req_buf.pop_back(); // pop out tu's FWD_REQ_V;
                    }
                }

                // Pending Transition from Expected State;
                else if (pending.u_state == DEV_OI)
                // Respond immediately;
                {
                    gen_reqor.msg = RSP_V;
                    req_buf.pop_back();
                }
            }
            else
            {
                if (data.state == SPX_O)
                // Just right in the expected state;
                {
                    gen_reqor.msg = RSP_V;
                    req_buf.pop_back();
                    // data.state = SPX_O;
                    // if(tu_id == CPU) tu_for_cpu(fwd_in);
                }
                else
                // In the Stable State other than Expected;
                {
                    MSG tmp = fwd_in;
                    gen_reqor.msg = RSP_NACK;
                    if (fwd_in.retry_times < MAX_RETRY)
                    {
                        tmp.retry_times = fwd_in.retry_times + 1;
                        req_buf.pop_back();
                        req_buf.push_back(tmp);
                        // fwd_in pop, tmp(another FWD_REQ_V) push in;
                    }
                    else
                    {
                        tmp.retry_times = 0;
                        tmp.msg = FWD_REQ_Odata; // FWD_WTfwd?
                        req_buf.pop_back();
                        req_buf.push_back(tmp);
                    }
                }
            }
            rsp_buf.push_back(gen_llc);
            rsp_buf.push_back(gen_reqor);
            break;
        }
    case FWD_REQ_O:
    {
        if (conflict_flag)
        {
            // Pending Transition to Expected State;
            // Respond immediately since no data transfer is needed, just ownership;
            if (pending.u_state == DEV_XO)
            {
                gen_reqor.msg = RSP_O;
                req_buf.pop_back();
            }

            // Pending Transition from Expected State;
            else if (pending.u_state == DEV_OI)
            {
                if (pending.msg == REQ_WB)
                {
                    llc.solve_pending_ReqWB(tu_id);
                    data.state = SPX_I;
                    gen_reqor.msg = RSP_O;
                    req_buf.pop_back();
                    // pop out FWD_REQ_O;
                    // REQ_WB waiting for RSP_NACK from LLC to pop out;
                }
            }
        }
        // Just right in the expected state;
        else if (data.state == SPX_O)
        {
            gen_reqor.msg = RSP_O;
            data.state = SPX_I;
            req_buf.pop_back();
        }
        rsp_buf.push_back(gen_llc);
        rsp_buf.push_back(gen_reqor);
        break;
    }
    case FWD_REQ_Odata:
    {
        if (conflict_flag)
        {
            // Pending Transition to Expected State;
            // that's how FWD_REQ_Odata diff from FWD_REQ_O;
            if (pending.u_state == DEV_XO)
            {
                // already push in;
                // just wait;
            }
            // Pending Transition from Expected State;
            else if (pending.u_state == DEV_OI)
            {
                if (pending.msg == REQ_WB)
                {
                    llc.solve_pending_ReqWB(tu_id);
                    data.state = SPX_I;
                    gen_reqor.msg = RSP_Odata;
                    req_buf.pop_back();
                    // pop out FWD_REQ_O;
                    // REQ_WB waiting for RSP_NACK from LLC to pop out;
                }
            }
        }
        // Just right in the expected state;
        else if (data.state == SPX_O)
        {
            gen_reqor.msg = RSP_Odata;
            data.state = SPX_I;
            req_buf.pop_back();
        }
        rsp_buf.push_back(gen_llc);
        rsp_buf.push_back(gen_reqor);
        break;
    }
    case FWD_RVK_O:
    {
        if (conflict_flag)
        {
            if (pending.u_state == DEV_XO)
            {
                // already push in;
                // just wait;
            }
            else if (pending.u_state == DEV_OI)
            {
                if (pending.msg == REQ_WB)
                {
                    llc.solve_pending_ReqWB(tu_id);
                    data.state = SPX_I;
                    gen_llc.msg = RSP_RVK_O;
                    req_buf.pop_back();
                }
            }
        }
        // Just right in the expected state;
        else if (data.state == SPX_O)
        {
            gen_llc.msg = RSP_RVK_O;
            data.state = SPX_I;
            req_buf.pop_back();
        }
        rsp_buf.push_back(gen_llc);
        // no rsp_buf.push_back(gen_reqor);
        break;
    }
    case FWD_INV:
    {
        if (conflict_flag)
        {
            if (pending.u_state == DEV_IS)
            {
                //???????? do not understand
            }
            else if (pending.u_state == DEV_XO)
            {
                //???????? do not understand
            }
        }
        // Just right in the expected state;
        else
        {
            if (data.state == SPX_S)
            {
                gen_llc.msg = RSP_INV_ACK;
                data.state = SPX_I;
                req_buf.pop_back();
            }
            else
            {
                gen_llc.msg = RSP_INV_ACK;
                // Why?
                req_buf.pop_back();
            }
        }
        rsp_buf.push_back(gen_llc);
        // no rsp_buf.push_back(gen_reqor);
        break;
    }
    case FWD_REQ_S:
    {
        if (conflict_flag)
        {
            if (pending.u_state == DEV_XO)
            {
                // already push in;
                // just wait;
            }
            else if (pending.u_state == DEV_OI)
            {
                if (pending.msg == REQ_WB)
                {
                    llc.solve_pending_ReqWB(tu_id);
                    data.state = SPX_S;
                    gen_reqor.msg = RSP_S;
                    req_buf.pop_back();
                }
            }
        }
        else if (data.state == SPX_O)
        {
            gen_reqor.msg = RSP_S;
            gen_llc.msg = RSP_RVK_O;
            data.state = SPX_S;
            req_buf.pop_back();
        }
        rsp_buf.push_back(gen_llc);
        rsp_buf.push_back(gen_reqor);
        break;
    }
    // case FWD_WTfwd:
    // {
    //     break;
    // }
    default:
        break;
    }
    WordIns(data, tu_line, offset);
}

// void TU::rcv_fwd_word(id_num_t &reqor_id, DEV &owner_dev, MSG &fwd_in)
// {
//     owner_dev.breakdown(fwd_in.addr);
//     owner_dev.fetch_line();
//     state_mapping(owner_dev.dev_id.to_ulong(), owner_dev.dev_line, owner_dev.dev_word);
//     rcv_fwd(reqor_id, fwd_in, fwd_in.offset, owner_dev.dev_line);
// }

void TU::rcv_fwd(id_num_t &reqor_id, DEV &owner_dev, MSG &fwd_in)
{
    owner_dev.breakdown(fwd_in.addr);
    owner_dev.fetch_line();
    state_mapping(owner_dev.dev_id.to_ulong(), owner_dev.dev_line, owner_dev.dev_word);

    bool flag = 0; // 0 for O; 1 for V;

    // 对于多字请求，应该触发1个回应，而不是2个回应;
    // e.g. Word1.3 in O, Word0.2 in V;  Word1.3 Rsp_a, Word0.2 Rsp_b;
    // 而非 Word0.1.2.3. Rsp_a.Rsp_b.Rsp_c.Rsp_d;

    // 很难做到…… 还是先拆分再合并吧;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (fwd_in.mask.test(i))
        {
            rcv_fwd_single(reqor_id, fwd_in, i);
        }
    }
    MsgCoalesce(rsp_buf);
    put_rsp(rsp_buf);
}

// void TU::rcv_fwd_cpu(id_num_t &reqor_id, MSG &fwd_in)
// // REQuired functionality for MESI TU (Section III-D).
// // CPU 中的数据总是以行组织的，但是外部请求可能以字到达，引发问题;
// {
//     MSG gen;
//     rcv_fwd(reqor_id, devs[CPU], fwd_in);

//     // Case 1: O state;
//     if (fwd_in.gran = GRAN_WORD)
//         gen.gran = GRAN_LINE;
//     // the only possible ownership downgrades for the whole line is FWD_REQ_S(REQ_S1) or FWD_RVK_O(REQWT+data);
//     else if (fwd_in.msg == FWD_REQ_S || fwd_in.msg == FWD_RVK_O)
//         gen.gran = GRAN_LINE;
//     // REQuired ownership downgrade for only part of the line FWD_REQ_O(REQWT);
//     // TU should trigger a REQWB for any non-downgraded words, other operations are handled by LLC or DEV themselves.
//     else if (fwd_in.msg == FWD_REQ_O)
//     {
//         gen.mask = ~fwd_in.mask; // non-downgraded words;
//         gen.msg = REQ_WB;
//     }
//     // Case 2: Pending O REQuest;
//     // else if (fwd_in.msg == REQ_V|| fwd_in.msg == REQ_S||fwd_in.msg == REQ_Odata||fwd_in.msg == FWD_RVK_O)
//     // Case 3: Pending write-back;
// }