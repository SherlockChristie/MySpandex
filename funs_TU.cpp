#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void TU::msg_init()
{
    for (int i = 0; i < MAX_DEVS; i++)
    {
        // rsp init;
        rsp_buf[i].msg = RSP_NULL;
        LineCopy(rsp_buf[i].data, tu_data.data_line);

        // req init;
        req_buf[i].msg = REQ_NULL;
        req_buf[i].gran = GRAN_WORD;
        // req_buf[i].addr = REQ.addr;
    }
}

void TU::req_mapping(unsigned long id, REQ &dev_req)
// Translate device message into LLC message (Table II).
{
    // REQ self_req = req_buf[id];
    // 不行，这样没有修改TU中req数组的值，还是得用指针;
    REQ *self_req = req_buf + id;
    self_req->addr = dev_req.addr;
    switch (id)
    {
    case GPU:
    {
        if (dev_req.msg == READ)
        {
            self_req->msg = REQ_V;
            self_req->gran = GRAN_LINE;
        }
        else if (dev_req.msg == WRITE)
        {
            self_req->msg = REQ_WT;
            self_req->gran = GRAN_WORD;
        }
        else if (dev_req.msg == RMW)
        {
            self_req->msg = REQ_WTdata;
            self_req->gran = GRAN_WORD;
        }
        break;
    }
    case ACC:
    {
        if (dev_req.msg == READ)
        {
            self_req->msg = REQ_V;
            self_req->gran = GRAN_WORD;
        }
        else if (dev_req.msg == WRITE)
        {
            self_req->msg = REQ_O;
            self_req->gran = GRAN_WORD;
        }
        else if (dev_req.msg == RMW)
        {
            self_req->msg = REQ_Odata;
            self_req->gran = GRAN_WORD;
        }
        else if (dev_req.msg == O_REPL)
        {
            self_req->msg = REQ_WB;
            self_req->gran = GRAN_WORD;
        }
        break;
    }
    case CPU:
    {
        if (dev_req.msg == READ)
        {
            self_req->msg = REQ_S;
            self_req->gran = GRAN_LINE;
        }
        else if (dev_req.msg == WRITE || dev_req.msg == RMW)
        {
            self_req->msg = REQ_Odata;
            self_req->gran = GRAN_LINE;
        }
        else if (dev_req.msg == O_REPL)
        {
            self_req->msg = REQ_WB;
            self_req->gran = GRAN_LINE;
        }
        break;
    }
    }
}

void TU::state_mapping(unsigned long id, DATA_LINE &dev_data)
// Translate device state into LLC state (Section III-D).
{
    bitset<STATE_LINE> state_line = BitSub<STATE_BITS, STATE_LINE>(dev_data.state, 0);
    bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(dev_data.state, 0);
    switch (id)
    {
    case GPU:
    {
        if (dev_data.state == DEV_I)
            state_line = LLC_I;
        else if (dev_data.state == DEV_V)
            state_line = LLC_V;
        break;
    }
    case ACC:
    {
        if (dev_data.state == DEV_I)
            state_line = LLC_I;
        else if (dev_data.state == DEV_V)
            state_line = LLC_V;
        else if (dev_data.state == DEV_O)
            state_line = LLC_O;
        break;
    }
    case CPU:
    {
        if (dev_data.state == DEV_I)
            state_line = LLC_I;
        else if (dev_data.state == DEV_S)
            state_line = LLC_S;
        else if (dev_data.state == DEV_M || dev_data.state == DEV_E)
            {state_line = LLC_V;
            for(int i = 0; i <WORDS_PER_LINE;i++) {state_line}}
        break;
    }
    }
}

void TU::mapping_wrapper(DEV &dev)
{
    unsigned long id = dev.id.to_ulong();
    req_mapping(id, dev.req_buf[id]);
    state_mapping(id, dev.dev_data);
}

void TU::tu_for_gpu()
// REQuired functionality for GPU coherence TU (Section III-D).
{
    unsigned long id = GPU;
    // support REQV retries;
    uint8_t retry_times;
    rsp_buf[id].msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        req_buf[id].msg = REQ_V;
    }
    else
    {
        req_buf[id].msg = REQ_WTdata;
    }
    //  handle partial word granularity responses;
    if (LineReady(state_line))
    {
        tu_callee_dev();
    }
    else
    {
        wait();
    }
}
void TU::tu_for_acc()
// REQuired functionality for DeNovo TU (Section III-D).
{
    unsigned long id = ACC;
    // support REQV retries;
    uint8_t retry_times;
    rsp_buf[id].msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        req_buf[id].msg = REQ_V;
    }
    else
    {
        req_buf[id].msg = REQ_WTdata;
        // or REQ_Odata?
    }
}

void TU::tu_for_cpu(TU &sender)
// REQuired functionality for MESI TU (Section III-D).
// CPU 中的数据总是以行组织的，但是外部请求可能以字到达，引发问题;
{
    unsigned long id = CPU;
    REQ sender_REQ = sender.req_buf[id];
    // REQ self_req = req_buf[id];
    REQ *self_req = req_buf + id;
    // Case 1: O state;
    if (sender_REQ.gran = GRAN_WORD)
        self_req->gran = GRAN_LINE;
    // the only possible ownership downgrades for the whole line is FWD_REQ_S(REQ_S1) or FWD_RVK_O(REQWT+data);
    else if (sender_REQ.msg == FWD_REQ_S || sender_REQ.msg == FWD_RVK_O)
        self_req->gran = GRAN_LINE;
    // REQuired ownership downgrade for only part of the line FWD_REQ_O(REQWT);
    // TU should trigger a REQWB for any non-downgraded words, other operations are handled by LLC or DEV themselves.
    else if (sender_REQ.msg == FWD_REQ_O)
    {
        self_req->mask = ~sender_REQ.mask; // non-downgraded words;
        self_req->msg = REQ_WB;
    }
    // Case 2: Pending O REQuest;
    // else if (sender_REQ.msg == REQ_V|| sender_REQ.msg == REQ_S||sender_REQ.msg == REQ_Odata||sender_REQ.msg == FWD_RVK_O)
    // Case 3: Pending write-back;
}

void TU::tu_callee_dev()
{
}