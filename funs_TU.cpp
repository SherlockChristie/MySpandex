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
    if(LineReady(tu_data.state))
    {
        
    }

}
void TU::tu_for_acc()
// Required functionality for DeNovo TU (Section III-D).
{
}
void TU::tu_for_cpu()
// Required functionality for MESI TU (Section III-D).
{
}

void TU::tu_callee_dev()
{
}