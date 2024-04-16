#include "classes.hpp"
#include "bit_utils.hpp"
using namespace std;

void TU::req_mapping(DEV &dev)
// Translate device message into LLC message (Table II).
{
    req.addr = dev.req.addr;
    switch (dev.type)
    {
    case GPU:
    {
        if (dev.req.dev_msg == READ)
        {
            req.tu_msg = REQ_V;
            req.gran = GRAN_LINE;
        }
        else if (dev.req.dev_msg == WRITE)
        {
            req.tu_msg = REQ_WT;
            req.gran = GRAN_WORD;
        }
        else if (dev.req.dev_msg == RMW)
        {
            req.tu_msg = REQ_WTdata;
            req.gran = GRAN_WORD;
        }
        break;
    }
    case ACC:
    {
        if (dev.req.dev_msg == READ)
        {
            req.tu_msg = REQ_V;
            req.gran = GRAN_WORD;
        }
        else if (dev.req.dev_msg == WRITE)
        {
            req.tu_msg = REQ_O;
            req.gran = GRAN_WORD;
        }
        else if (dev.req.dev_msg == RMW)
        {
            req.tu_msg = REQ_Odata;
            req.gran = GRAN_WORD;
        }
        else if (dev.req.dev_msg == O_REPL)
        {
            req.tu_msg = REQ_WB;
            req.gran = GRAN_WORD;
        }
        break;
    }
    case CPU:
    {
        if (dev.req.dev_msg == READ)
        {
            req.tu_msg = REQ_S;
            req.gran = GRAN_LINE;
        }
        else if (dev.req.dev_msg == WRITE || dev.req.dev_msg == RMW)
        {
            req.tu_msg = REQ_Odata;
            req.gran = GRAN_LINE;
        }
        else if (dev.req.dev_msg == O_REPL)
        {
            req.tu_msg = REQ_WB;
            req.gran = GRAN_LINE;
        }
        break;
    }
    }
}

void TU::state_mapping(DEV &dev)
// Translate device state into LLC state (Section III-D).
{
    switch (dev.type)
    {
    case GPU:
    {
        if (dev.dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev.dev_data.state == DEV_V)
            tu_data.state = SPX_V;
        break;
    }
    case ACC:
    {
        if (dev.dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev.dev_data.state == DEV_V)
            tu_data.state = SPX_V;
        else if (dev.dev_data.state == DEV_O)
            tu_data.state = SPX_O;
        break;
    }
    case CPU:
    {
        if (dev.dev_data.state == DEV_I)
            tu_data.state = SPX_I;
        else if (dev.dev_data.state == DEV_S)
            tu_data.state = SPX_S;
        else if (dev.dev_data.state == DEV_M || dev.dev_data.state == DEV_E)
            tu_data.state = SPX_O;
        break;
    }
    }
}

void TU::tu_for_gpu()
// Required functionality for GPU coherence TU (Section III-D).
{
    // support ReqV retries;
    uint8_t retry_times;
    rsp.tu_msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        req.tu_msg = REQ_V;
    }
    else
    {
        req.tu_msg = REQ_WTdata;
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
// Required functionality for DeNovo TU (Section III-D).
{
    // support ReqV retries;
    uint8_t retry_times;
    rsp.tu_msg = RSP_NACK;
    if (retry_times < MAX_RETRY)
    {
        req.tu_msg = REQ_V;
    }
    else
    {
        req.tu_msg = REQ_WTdata;
        // or REQ_Odata?
    }
}

void TU::tu_for_cpu(TU &sender)
// Required functionality for MESI TU (Section III-D).
// CPU 中的数据总是以行组织的，但是外部请求可能以字到达，引发问题;
{
    // Case 1: O state;
    if (sender.req.gran = GRAN_WORD)
        req.gran = GRAN_LINE;
    // the only possible ownership downgrades for the whole line is FWD_REQ_S(REQ_S1) or FWD_RVK_O(ReqWT+data);
    else if (sender.req.tu_msg == FWD_REQ_S || sender.req.tu_msg == FWD_RVK_O)
        req.gran = GRAN_LINE;
    // required ownership downgrade for only part of the line FWD_REQ_O(ReqWT);
    // TU should trigger a ReqWB for any non-downgraded words, other operations are handled by LLC or DEV themselves.
    else if (sender.req.tu_msg == FWD_REQ_O)
    {
        req.mask = ~sender.req.mask; // non-downgraded words;
        req.tu_msg = REQ_WB;
    }
    // Case 2: Pending O request;
    // else if (sender.req.tu_msg == REQ_V|| sender.req.tu_msg == REQ_S||sender.req.tu_msg == REQ_Odata||sender.req.tu_msg == FWD_RVK_O)
    // Case 3: Pending write-back;
}

void TU::tu_callee_dev()
{
}