#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void TU::msg_init()
{
    for (int i = 0; i < MAX_DEVS; i++)
    {
        // RSP init;
        rsp_buf[i].tu_msg = RSP_NULL;
        LineCopy(rsp_buf[i].data, tu_data.data_line);

        // REQ init;
        req_buf[i].tu_msg = REQ_NULL;
        req_buf[i].gran = GRAN_WORD;
        req_buf[i].addr = TU_REQ.addr;
    }
}

void TU::REQ_mapping(unsigned long id, DEV_REQ &DEV_REQ)
// Translate device message into LLC message (Table II).
{
    // TU_REQ self_REQ = REQ[id];
    // 不行，这样没有修改TU中REQ数组的值，还是得用指针;
    TU_REQ *self_REQ = REQ + id;
    self_REQ->addr = DEV_REQ.addr;
    switch (id)
    {
    case GPU:
    {
        if (DEV_REQ.dev_msg == READ)
        {
            self_REQ->tu_msg = REQ_V;
            self_REQ->gran = GRAN_LINE;
        }
        else if (DEV_REQ.dev_msg == WRITE)
        {
            self_REQ->tu_msg = REQ_WT;
            self_REQ->gran = GRAN_WORD;
        }
        else if (DEV_REQ.dev_msg == RMW)
        {
            self_REQ->tu_msg = REQ_WTdata;
            self_REQ->gran = GRAN_WORD;
        }
        break;
    }
    case ACC:
    {
        if (DEV_REQ.dev_msg == READ)
        {
            self_REQ->tu_msg = REQ_V;
            self_REQ->gran = GRAN_WORD;
        }
        else if (DEV_REQ.dev_msg == WRITE)
        {
            self_REQ->tu_msg = REQ_O;
            self_REQ->gran = GRAN_WORD;
        }
        else if (DEV_REQ.dev_msg == RMW)
        {
            self_REQ->tu_msg = REQ_Odata;
            self_REQ->gran = GRAN_WORD;
        }
        else if (DEV_REQ.dev_msg == O_REPL)
        {
            self_REQ->tu_msg = REQ_WB;
            self_REQ->gran = GRAN_WORD;
        }
        break;
    }
    case CPU:
    {
        if (DEV_REQ.dev_msg == READ)
        {
            self_REQ->tu_msg = REQ_S;
            self_REQ->gran = GRAN_LINE;
        }
        else if (DEV_REQ.dev_msg == WRITE || DEV_REQ.dev_msg == RMW)
        {
            self_REQ->tu_msg = REQ_Odata;
            self_REQ->gran = GRAN_LINE;
        }
        else if (DEV_REQ.dev_msg == O_REPL)
        {
            self_REQ->tu_msg = REQ_WB;
            self_REQ->gran = GRAN_LINE;
        }
        break;
    }
    }
}

void TU::state_mapping(unsigned long id, DEV_DATA &dev_data)
// Translate device state into LLC state (Section III-D).
{
    switch (id)
    {
    case GPU:
    {
        if (dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev_data.state == DEV_V)
            tu_data.state = SPX_V;
        break;
    }
    case ACC:
    {
        if (dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev_data.state == DEV_V)
            tu_data.state = SPX_V;
        else if (dev_data.state == DEV_O)
            tu_data.state = SPX_O;
        break;
    }
    case CPU:
    {
        if (dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev_data.state == DEV_S)
            tu_data.state = SPX_S;
        else if (dev_data.state == DEV_M || dev_data.state == DEV_E)
            tu_data.state = SPX_O;
        break;
    }
    }
}

void TU::mapping_wrapper(DEV &dev)
{
    unsigned long id = dev.id.to_ulong();
    REQ_mapping(id, dev.REQ[id]);
    state_mapping(id, dev.dev_data);
}

void TU::tu_for_gpu()
// REQuired functionality for GPU coherence TU (Section III-D).
{
    unsigned long id = GPU;
    // support REQV retries;
    uint8_t retry_times;
    RSP[id].tu_msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        REQ[id].tu_msg = REQ_V;
    }
    else
    {
        REQ[id].tu_msg = REQ_WTdata;
    }
    //  handle partial word granularity responses;
    if (LineReady(tu_data.state))
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
    RSP[id].tu_msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        REQ[id].tu_msg = REQ_V;
    }
    else
    {
        REQ[id].tu_msg = REQ_WTdata;
        // or REQ_Odata?
    }
}

void TU::tu_for_cpu(TU &sender)
// REQuired functionality for MESI TU (Section III-D).
// CPU 中的数据总是以行组织的，但是外部请求可能以字到达，引发问题;
{
    unsigned long id = CPU;
    TU_REQ sender_REQ = sender.REQ[id];
    // TU_REQ self_REQ = REQ[id];
    TU_REQ *self_REQ = REQ + id;
    // Case 1: O state;
    if (sender_REQ.gran = GRAN_WORD)
        self_REQ->gran = GRAN_LINE;
    // the only possible ownership downgrades for the whole line is FWD_REQ_S(REQ_S1) or FWD_RVK_O(REQWT+data);
    else if (sender_REQ.tu_msg == FWD_REQ_S || sender_REQ.tu_msg == FWD_RVK_O)
        self_REQ->gran = GRAN_LINE;
    // REQuired ownership downgrade for only part of the line FWD_REQ_O(REQWT);
    // TU should trigger a REQWB for any non-downgraded words, other operations are handled by LLC or DEV themselves.
    else if (sender_REQ.tu_msg == FWD_REQ_O)
    {
        self_REQ->mask = ~sender_REQ.mask; // non-downgraded words;
        self_REQ->tu_msg = REQ_WB;
    }
    // Case 2: Pending O REQuest;
    // else if (sender_REQ.tu_msg == REQ_V|| sender_REQ.tu_msg == REQ_S||sender_REQ.tu_msg == REQ_Odata||sender_REQ.tu_msg == FWD_RVK_O)
    // Case 3: Pending write-back;
}

void TU::tu_callee_dev()
{
}