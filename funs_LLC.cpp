#include "classes.hpp"
#include "bit_utils.hpp"

void LLC::breakdown(LLC_ADDR &llc_addr, addr_t addr)
{
    llc_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    llc_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    llc_addr.index = BitSub<ADDR_SIZE, LLC_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    llc_addr.tag = BitSub<ADDR_SIZE, LLC_TAG_BITS>(addr, LLC_INDEX_BITS + WORDS_OFF + BYTES_OFF);
};

void LLC::fetch_line(LLC_ADDR &llc_addr, LLC_DATA &llc_data)
{
    unsigned long llc_index = (llc_addr.index).to_ulong();
    line_t temp = {0};
    llc_data.state = state_buf[llc_index];
    llc_data.sharers = sharers_buf[llc_index];
    if ((tag_buf[llc_index] != llc_addr.tag) || (llc_data.state == LLC_I))
    {
        llc_data.hit = 0;
        // line_t *p = (line_t *)llc_data.data_line;
        // LineCopy(*p,temp);
        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            for (int j = 0; j < BYTES_PER_WORD; j++)
            {
                llc_data.data_line[i][j] = 0;
            }
        }
    }
    else
    {
        llc_data.hit = 1;
        for (int i = 0; i < WORDS_PER_LINE; i++)
        {
            for (int j = 0; j < BYTES_PER_WORD; j++)
            {
                llc_data.data_line[i][j] = cache[llc_index][i * BYTES_PER_WORD + j];
            }
        }
    }
};

// not needed
// void LLC::dev_lookup_in_llc(addr_t dev_addr)
// {
//     // Input: TU req addr(same with DEV addr).
//     // Goal: Find whether the data dev missed is in llc or not.
//     bool tag_hit = 0;

//     // Step 1: translate the address into llc's type.
//     LLC_ADDR llc_addr;
//     llc_addr.breakdown(dev_addr);

//     // llc_index_t index_mask = (1 << LLC_INDEX_BITS) - 1;
//     // llc_addr.index = CatBit(dev_addr->tag, dev_addr->index);
//     // llc_addr.index = ((dev_addr->tag)<<(DEV_INDEX_BITS))|(dev_addr->index);

//     // Step 2: Compare.
//     // NOTE that operate [] needs an int here, so convert bitset llc_addr.index into int first.
//     unsigned long index = (llc_addr.index).to_ulong();
//     if (tag_buf[index] == llc_addr.tag && llc_state != LLC_I)
//     {
//         tag_hit = 1;
//     }
// };

void LLC::forwards(TU &owner){};

void LLC::rcv_req(TU &tu)
// Behaviour when LLC receives an external request from TU (Table III).
{
    breakdown(llc_addr, tu.req.addr);
    fetch_line(llc_addr, llc_data);

    TU owner;

    // if (!llc_data.hit)
    // {
    //     // go to main memory;
    // }
    // else
    {
        switch (tu.req.tu_msg)
        {
        case REQ_V:
        {
            if (llc_data.state == LLC_V || llc_data.state == LLC_S)
            {
            }
            else if (llc_data.state == LLC_O)
            {
                req.llc_msg = FWD_REQ_V;
                forwards(owner);
            };
            break;
        }
        case REQ_S:
            // ReqS2 not used;
            {
                if (llc_data.state == LLC_S) // ReqS1;
                {
                    // no blocking state to S;
                    llc_data.state = LLC_S;
                }
                else if (llc_data.state == LLC_O)
                {
                    if (owner.type == CPU) // ReqS1;
                    {
                        // having blocking states;
                        llc_data.state = LLC_OS;
                        req.llc_msg = FWD_REQ_S;
                    }
                    else // ReqS3;
                    {
                        llc_data.state = LLC_O;
                        req.llc_msg = FWD_REQ_Odata;
                    }
                    forwards(owner);
                }
                // ReqS3;
                else if (llc_data.state == LLC_V)
                {
                    llc_data.state = LLC_O;
                }
                break;
            }
        case REQ_WT:
        {
            if (llc_data.state == LLC_O)
            {
                req.llc_msg = FWD_REQ_O;
                forwards(owner);
            }
            llc_data.state = LLC_V;
            break;
        }
        case REQ_O:
        {
            if (llc_data.state == LLC_O)
            {
                req.llc_msg = FWD_REQ_O;
                forwards(owner);
            }
            llc_data.state = LLC_O;
            break;
        }
        case REQ_WTdata:
        {
            if (llc_data.state == LLC_O)
            {
                req.llc_msg = FWD_RVK_O;
                forwards(owner);
                // having blocking states;
                llc_data.state = LLC_OV;
            }
            else if (llc_data.state == LLC_V)
            {
            }
            else if (llc_data.state == LLC_S)
            {
                // having blocking states;
                llc_data.state = LLC_SV;
            }
            break;
        }
        case REQ_Odata:
        {

            if (llc_data.state == LLC_O)
            {
                req.llc_msg = FWD_REQ_Odata;
                forwards(owner);
                // llc_data.state == LLC_O; // no blocking states;
            }
            else if (llc_data.state == LLC_V)
            {
                // no blocking states;
                llc_data.state == LLC_O;
            }
            else if (llc_data.state == LLC_S)
            {
                // having blocking states;
                llc_data.state = LLC_SO;
            }
            break;
        }
        case REQ_WB:
        {
            if (tu.type == owner.type)
            {
                llc_data.state == LLC_V;
            }
            else
            {
                // invalid operation, no response;
            }
            break;
        }
        }
    }
};