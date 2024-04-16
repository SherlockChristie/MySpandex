#include "classes.hpp"
#include "bit_utils.hpp"

void LLC::msg_init()
{
    for (int i = 0; i < MAX_DEVS; i++)
    {
        // if(i==SPX) exit;
        // else 
        // do not need, since SPX still needs RSP_NULL/REQ_NULL to indicate that this is not used.

        // RSP init;
        rsp[i].llc_msg = RSP_NULL;
        LineCopy(rsp[i].data, llc_data.data_line);

        // REQ init;
        req[i].llc_msg = REQ_NULL;
        req[i].gran = GRAN_WORD;
        // req[i].addr = TU_REQ.addr;
    }
}

void LLC::breakdown(LLC_ADDR &llc_addr, addr_t addr)
{
    llc_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    llc_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    llc_addr.index = BitSub<ADDR_SIZE, LLC_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    llc_addr.tag = BitSub<ADDR_SIZE, LLC_TAG_BITS>(addr, LLC_INDEX_BITS + WORDS_OFF + BYTES_OFF);
}

bool LLC::fetch_line(LLC_ADDR &llc_addr, LLC_DATA &llc_data)
{
    line_t zero = {0};
    unsigned long llc_index = (llc_addr.index).to_ulong();
    llc_data.state = state_buf[llc_index];
    llc_data.sharers = sharers_buf[llc_index];
    if ((tag_buf[llc_index] != llc_addr.tag) || (llc_data.state == LLC_I))
    {
        LineCopy(llc_data.data_line, zero);
        return 0;
        // line_t *p = (line_t *)llc_data.data_line;
        // LineCopy(*p,temp);
        // for (int i = 0; i < WORDS_PER_LINE; i++)
        // {
        //     for (int j = 0; j < BYTES_PER_WORD; j++)
        //     {
        //         llc_data.data_line[i][j] = 0;
        //     }
        // }
    }
    else
    {
        LineCopy(llc_data.data_line, cache[llc_index]);
        return 1;
        // for (int i = 0; i < WORDS_PER_LINE; i++)
        // {
        //     for (int j = 0; j < BYTES_PER_WORD; j++)
        //     {
        //         llc_data.data_line[i][j] = cache[llc_index][i * BYTES_PER_WORD + j];
        //     }
        // }
    }
}

// not needed
// void LLC::dev_lookup_in_llc(addr_t dev_addr)
// {
//     // Input: TU REQ addr(same with DEV addr).
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

id_t LLC::find_owner(LLC_DATA &llc_data)
{
    word_t owner_word;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (llc_data.state.test(i))
            WordExt(owner_word, i, llc_data.data_line);
    }
    id_t owner_bits(owner_word);
    return owner_bits;
}

void LLC::rcv_REQ(id_t &tu_id, TU_REQ &TU_REQ)
// Behaviour when LLC receives an external REQuest from TU (Table III).
{
    unsigned long id = tu_id.to_ullong();
    breakdown(llc_addr, TU_REQ.addr);
    fetch_line(llc_addr, llc_data);
    msg_init();
    REQ[id].addr = TU_REQ.addr;
    // Default address: the REQ's addr.


    // if (!llc_data.hit)
    // {
    //     // missed, go to main memory;
    // }
    // else
    {
        switch (TU_REQ.tu_msg)
        {
        case REQ_V:
        {
            if (llc_data.state == LLC_V || llc_data.state == LLC_S)
            {
                RSP[0].llc_msg = RSP_V;
            }
            else if (llc_data.state == LLC_O)
            {
                REQ[0].llc_msg = FWD_REQ_V;
                REQ[0].dest = find_owner(llc_data);
            };
            break;
        }
        case REQ_S:
            // REQS2 not used;
            {
                llc_data.sharers.set(tu_id.to_ulong()); // Update the sharers list.
                if (llc_data.state == LLC_S)            // REQS1;
                {
                    // no blocking state to S;
                    llc_data.state = LLC_S;
                    RSP[0].llc_msg = RSP_S;
                }
                else if (llc_data.state == LLC_O)
                {
                    REQ[0].dest = find_owner(llc_data);
                    if (REQ[0].dest == CPU) // REQS1;
                    {
                        // having blocking states;
                        llc_data.state = LLC_OS;
                        REQ[0].llc_msg = FWD_REQ_S;
                    }
                    else // REQS3;
                    {
                        llc_data.state = LLC_O;
                        REQ[0].llc_msg = FWD_REQ_Odata;
                    }
                }
                // REQS3;
                else if (llc_data.state == LLC_V)
                {
                    llc_data.state = LLC_O;
                    RSP[0].llc_msg = RSP_S;
                }
                break;
            }
        case REQ_WT:
        {
            if (llc_data.state == LLC_O)
            {
                REQ[0].llc_msg = FWD_REQ_O;
                REQ[0].dest = find_owner(llc_data);
                llc_data.state = LLC_V;
            }
            else if (llc_data.state == LLC_V)
            {
                RSP[0].llc_msg = RSP_WT;
                llc_data.state = LLC_V;
            }
            else if (llc_data.state == LLC_S)
            {
                llc_data.state = LLC_SV; // go to blocking states;
                // wait();
                for (int i = 1; i < MAX_DEVS; i++)
                // LLC itself does not need a forward message, so i starts from 1;
                {
                    if (llc_data.sharers.test(i))
                    {
                        req[i].llc_msg = FWD_INV;
                        req[i].dest = bitset<MAX_DEVS_BITS>(i);
                    }
                }
            }
            break;
        }
        case REQ_O:
        {
            if (llc_data.state == LLC_O)
            {
                REQ[0].llc_msg = FWD_REQ_O;
                REQ[0].dest = find_owner(llc_data);
                // llc_data.state = LLC_O;
            }
            else if (llc_data.state == LLC_V)
            {
                RSP[0].llc_msg = RSP_O;
                llc_data.state = LLC_O;
            }
            else if (llc_data.state == LLC_S)
            {
                llc_data.state = LLC_SO; // go to blocking states;
                // wait();
                for (int i = 1; i < MAX_DEVS; i++)
                // LLC itself does not need a forward message, so i starts from 1;
                {
                    if (llc_data.sharers.test(i))
                    {
                        req[i].llc_msg = FWD_INV;
                        req[i].dest = bitset<MAX_DEVS_BITS>(i);
                    }
                }
            }
            break;
        }
        case REQ_WTdata:
        {
            if (llc_data.state == LLC_O)
            {
                REQ[0].llc_msg = FWD_RVK_O;
                REQ[0].dest = find_owner(llc_data);
                // having blocking states;
                llc_data.state = LLC_OV;
            }
            else if (llc_data.state == LLC_V)
            {
                RSP[0].llc_msg = RSP_WTdata;
            }
            else if (llc_data.state == LLC_S)
            {
                // having blocking states;
                llc_data.state = LLC_SV;
                // wait();
                for (int i = 1; i < MAX_DEVS; i++)
                // LLC itself does not need a forward message, so i starts from 1;
                {
                    if (llc_data.sharers.test(i))
                    {
                        req[i].llc_msg = FWD_INV;
                        req[i].dest = bitset<MAX_DEVS_BITS>(i);
                    }
                }
            }
            break;
        }
        case REQ_Odata:
        {
            if (llc_data.state == LLC_O)
            {
                REQ[0].llc_msg = FWD_REQ_Odata;
                REQ[0].dest = find_owner(llc_data);
                // llc_data.state == LLC_O; // no blocking states;
            }
            else if (llc_data.state == LLC_V)
            {
                // no blocking states;
                llc_data.state == LLC_O;
                RSP[0].llc_msg = RSP_Odata;
            }
            else if (llc_data.state == LLC_S)
            {
                // having blocking states;
                llc_data.state = LLC_SO;
                // wait();
                for (int i = 1; i < MAX_DEVS; i++)
                // LLC itself does not need a forward message, so i starts from 1;
                {
                    if (llc_data.sharers.test(i))
                    {
                        req[i].llc_msg = FWD_INV;
                        req[i].dest = bitset<MAX_DEVS_BITS>(i);
                    }
                }
            }
            break;
        }
        case REQ_WB:
        {
            if (llc_data.state == LLC_O)
            {
                REQ[0].dest = find_owner(llc_data);
                if (tu_id == REQ[0].dest)
                {
                    llc_data.state == LLC_V;
                    RSP[0].llc_msg = RSP_WB_ACK;
                }
                else
                {
                    // invalid operation for the non-owner to write;
                    RSP[0].llc_msg = RSP_NACK;
                }
            }
            // is that possible for a REQ_WB in LLC_S???
            else if (llc_data.state == LLC_S)
            {
                // having blocking states;
                llc_data.state = LLC_SV;
                // wait();
                for (int i = 1; i < MAX_DEVS; i++)
                // LLC itself does not need a forward message, so i starts from 1;
                {
                    if (llc_data.sharers.test(i))
                    {
                        req[i].llc_msg = FWD_INV;
                        req[i].dest = bitset<MAX_DEVS_BITS>(i);
                    }
                }
            }
            break;
        }
        }
    }
}

void LLC::snd_REQ() {}

void LLC::snd_RSP() {}