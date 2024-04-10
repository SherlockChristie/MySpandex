#include "classes.hpp"
#include <bitset>

void breakdown(LLC_ADDR &llc_addr,addr_t addr)
{
    llc_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    llc_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    llc_addr.index = BitSub<ADDR_SIZE, LLC_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    llc_addr.tag = BitSub<ADDR_SIZE, LLC_TAG_BITS>(addr, LLC_INDEX_BITS + WORDS_OFF + BYTES_OFF);
};

void fetch_line(LLC &llc, LLC_ADDR &llc_addr,LLC_DATA &llc_data)
{
    unsigned long llc_index = (llc_addr.index).to_ulong();
    llc_data.state = llc.state_buf[llc_index];
    llc_data.sharers = llc.sharers_buf[llc_index];
    if ((llc.tag_buf[llc_index] != llc_addr.tag) || (state == LLC_I))
    {
        llc_data.hit = 0;
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
                llc_data.data_line[i][j] = llc.cache[llc_index][i * BYTES_PER_WORD + j];
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

void LLC::rcv_req(TU_REQ &req)
// Behaviour when LLC receives an external request from TU (Table III).
{
    LLC_ADDR llc_addr;
    breakdown(llc_addr,req.addr);
    LLC_DATA llc_data;
    fetch_line(llc_data, llc_addr);
    unsigned long llc_index = (llc_addr.index).to_ulong();
    state_t llc_state = state_buf[llc_index];

    if ((tag_buf[llc_index] != llc_addr.tag) || (llc_state == LLC_I))
    {
        // go to main memory;
    }
    else
    {
        switch (req.tu_msg)
        {
        case REQ_V:
        {
            if (llc_state == LLC_V || llc_state == LLC_S)
            {
            };
            break;
        }
        case REQ_S:
        {

            break;
        }
        case REQ_WT:
        {
            break;
        }
        case REQ_O:
        {
            break;
        }
        case REQ_WTdata:
        {
            break;
        }
        case REQ_Odata:
        {
            break;
        }
        case REQ_WB:
        {
            break;
        }
        }
    }
};