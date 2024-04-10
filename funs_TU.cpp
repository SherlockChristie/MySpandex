#include "classes.hpp"
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
};

void TU::state_mapping(){};