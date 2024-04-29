#include "blocks.hpp"

string msg_which(int msg)
{
    switch (msg)
    {
    case REQ_S:
        return "REQ_S";

    case REQ_Odata:
        return "REQ_Odata";

    case REQ_WT:
        return "REQ_WT";

    case REQ_WB:
        return "REQ_WB";

    case REQ_O:
        return "REQ_O";

    case REQ_V:
        return "REQ_V";

    case REQ_WTdata:
        return "REQ_WTdata";

    case REQ_WTfwd:
        return "REQ_WTfwd";

    case FWD_REQ_S:
        return "FWD_REQ_S";

    case FWD_REQ_Odata:
        return "FWD_REQ_Odata";

    case FWD_INV:
        return "FWD_INV";

    case FWD_WB_ACK:
        return "FWD_WB_ACK";

    case FWD_RVK_O:
        return "FWD_RVK_O";

    case FWD_REQ_V:
        return "FWD_REQ_V";

    case FWD_REQ_O:
        return "FWD_REQ_O";

    case FWD_WTfwd:
        return "FWD_WTfwd";

    case RSP_S:
        return "RSP_S";

    case RSP_Odata:
        return "RSP_Odata";

    case RSP_INV_ACK:
        return "RSP_INV_ACK";

    case RSP_NACK:
        return "RSP_NACK";

    case RSP_RVK_O:
        return "RSP_RVK_O";

    case RSP_V:
        return "RSP_V";

    case RSP_O:
        return "RSP_O";

    case RSP_WT:
        return "RSP_WT";

    case RSP_WTdata:
        return "RSP_WTdata";

    case RSP_WB_ACK:
        return "RSP_WB_ACK";

    case RSP_FWD:
        return "RSP_FWD";

    default:
        return "NOT FOUND";
    }
}


string dev_which(int id)
{
    switch (id)
    {
    case SPX:
        return "LLC";

    case CPU:
        return "CPU";

    case GPU:
        return "GPU";

    case ACC:
        return "ACC";

    default:
        return "NOT FOUND";
    }
}