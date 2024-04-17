#include "classes.hpp"
#include "bit_utils.hpp"

// void LLC::msg_init()
// {
//     for (int i = 0; i < MAX_DEVS; i++)
//     {
//         // if(i==SPX) exit;
//         // else
//         // do not need, since SPX still needs RSP_NULL/REQ_NULL to indicate that this is not used.

//         // rsp init;
//         rsp_buf[i].msg = RSP_NULL;
//         LineCopy(rsp_buf[i].data, llc_data.data_line);

//         // req init;
//         rsp_buf[i].msg = REQ_NULL;
//         rsp_buf[i].gran = GRAN_WORD;
//         // rsp_buf[i].addr = REQ.addr;
//     }
// }

void LLC::breakdown(LLC_ADDR &llc_addr, addr_t addr)
{
    llc_addr.b_off = BitSub<ADDR_SIZE, BYTES_OFF>(addr, 0);
    llc_addr.w_off = BitSub<ADDR_SIZE, WORDS_OFF>(addr, BYTES_OFF);
    llc_addr.index = BitSub<ADDR_SIZE, LLC_INDEX_BITS>(addr, WORDS_OFF + BYTES_OFF);
    llc_addr.tag = BitSub<ADDR_SIZE, LLC_TAG_BITS>(addr, LLC_INDEX_BITS + WORDS_OFF + BYTES_OFF);
}

bool LLC::fetch_line(LLC_ADDR &llc_addr, DATA_LINE &llc_data)
{
    line_t zero = {0};
    unsigned long llc_index = (llc_addr.index).to_ulong();
    llc_data.state = state_buf[llc_index];
    llc_data.sharers = sharers_buf[llc_index];
    if ((tag_buf[llc_index] != llc_addr.tag) || (llc_data.state == LLC_I))
    {
        LineCopy(llc_data.data, zero);
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
        LineCopy(llc_data.data, cache[llc_index]);
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
//     SPX_ADDR llc_addr;
//     llc_addr.breakdown(dev_addr);

//     // llc_index_t index_mask = (1 << SPX_INDEX_BITS) - 1;
//     // llc_addr.index = CatBit(dev_addr->tag, dev_addr->index);
//     // llc_addr.index = ((dev_addr->tag)<<(DEV_INDEX_BITS))|(dev_addr->index);

//     // Step 2: Compare.
//     // NOTE that operate [] needs an int here, so convert bitset llc_addr.index into int first.
//     unsigned long index = (llc_addr.index).to_ulong();
//     if (tag_buf[index] == llc_addr.tag && llc_state != SPX_I)
//     {
//         tag_hit = 1;
//     }
// };

void LLC::rcv_req(id_t &tu_id, MSG &tu_req, int rsp_count, word_offset_t mask, DATA_LINE &llc_data)
// Behaviour when LLC receives an external request from TU (Table III).
{
    DATA_WORD data;
    WordExt(data, llc_data, mask);
    // breakdown(llc_addr, tu_req.addr);
    // fetch_line(llc_addr, llc_data);
    // msg_init();

    rsp_buf[rsp_count].dest = tu_id;
    // Default destination: the requestor.
    rsp_buf[rsp_count].addr = tu_req.addr;
    // Default address: the req's addr.
    rsp_buf[rsp_count].gran = GRAN_WORD;
    // Default LLC granularity: word.
    rsp_buf[rsp_count].mask = mask;
    // Default mask.
    rsp_buf[rsp_count].data_line = llc_data;
    rsp_buf[rsp_count].data_word = data;
    // Default data.
    // msg and u_state is decided below.

    // if (!llc_data.hit)
    // {
    //     // missed, go to main memory;
    // }
    // else
    {
        switch (tu_req.msg)
        {
        case REQ_V:
        {
            if (data.state == SPX_V || data.state == SPX_S)
            {
                rsp_buf[rsp_count].msg = RSP_V;
            }
            else if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].msg = FWD_REQ_V;
                rsp_buf[rsp_count].dest = find_owner(llc_data);
            };
            break;
        }
        case REQ_S:
            // REQS2 not used;
            {
                llc_data.sharers.set(tu_id.to_ulong()); // Update the sharers list.
                if (data.state == SPX_S)                // REQS1;
                {
                    // no blocking state to S;
                    data.state = SPX_S;
                    rsp_buf[rsp_count].msg = RSP_S;
                }
                else if (data.state == SPX_O)
                {
                    rsp_buf[rsp_count].dest = find_owner(llc_data);
                    if (rsp_buf[rsp_count].dest == CPU) // REQS1;
                    {
                        // having blocking states;
                        tu_req.u_state = LLC_OS;
                        rsp_buf[rsp_count].msg = FWD_REQ_S;
                    }
                    else // REQS3;
                    {
                        data.state = SPX_O;
                        rsp_buf[rsp_count].msg = FWD_REQ_Odata;
                    }
                }
                // REQS3;
                else if (data.state == SPX_V)
                {
                    data.state = SPX_O;
                    rsp_buf[rsp_count].msg = RSP_S;
                }
                break;
            }
        case REQ_WT:
        {
            if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].msg = FWD_REQ_O;
                rsp_buf[rsp_count].dest = find_owner(llc_data);
                data.state = SPX_V;
            }
            else if (data.state == SPX_V)
            {
                rsp_buf[rsp_count].msg = RSP_WT;
                data.state = SPX_V;
            }
            else if (data.state == SPX_S)
            {
                tu_req.u_state = LLC_SV; // go to blocking states;
                // wait();
                rsp_buf[rsp_count].msg = FWD_INV;
                InvSharers(llc_data.sharers, SPX, rsp_buf[rsp_count].dest);
            }
            break;
        }
        case REQ_O:
        {
            if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].msg = FWD_REQ_O;
                rsp_buf[rsp_count].dest = find_owner(llc_data);
                // data.state = SPX_O;
            }
            else if (data.state == SPX_V)
            {
                rsp_buf[rsp_count].msg = RSP_O;
                data.state = SPX_O;
            }
            else if (data.state == SPX_S)
            {
                tu_req.u_state = LLC_SO; // go to blocking states;
                // Pay attetion: Unstable state go to the req triggers it;
                // wait();
                rsp_buf[rsp_count].msg = FWD_INV;
                InvSharers(llc_data.sharers, SPX, rsp_buf[rsp_count].dest);
            }
            break;
        }
        case REQ_WTdata:
        {
            if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].msg = FWD_RVK_O;
                rsp_buf[rsp_count].dest = find_owner(llc_data);
                // having blocking states;
                tu_req.u_state = LLC_OV;
            }
            else if (data.state == SPX_V)
            {
                rsp_buf[rsp_count].msg = RSP_WTdata;
            }
            else if (data.state == SPX_S)
            {
                // having blocking states;
                tu_req.u_state = LLC_SV;
                // wait();
                rsp_buf[rsp_count].msg = FWD_INV;
                InvSharers(llc_data.sharers, SPX, rsp_buf[rsp_count].dest);
            }
            break;
        }
        case REQ_Odata:
        {
            if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].msg = FWD_REQ_Odata;
                rsp_buf[rsp_count].dest = find_owner(llc_data);
                // data.state == SPX_O; // no blocking states;
            }
            else if (data.state == SPX_V)
            {
                // no blocking states;
                data.state == SPX_O;
                rsp_buf[rsp_count].msg = RSP_Odata;
            }
            else if (data.state == SPX_S)
            {
                // having blocking states;
                tu_req.u_state = LLC_SO;
                // wait();
                rsp_buf[rsp_count].msg = FWD_INV;
                InvSharers(llc_data.sharers, SPX, rsp_buf[rsp_count].dest);
            }
            break;
        }
        case REQ_WB:
        {
            if (data.state == SPX_O)
            {
                rsp_buf[rsp_count].dest = find_owner(llc_data);
                if (tu_id == rsp_buf[rsp_count].dest)
                {
                    data.state == SPX_V;
                    rsp_buf[rsp_count].msg = RSP_WB_ACK;
                }
                else
                {
                    // invalid operation for the non-owner to write;
                    rsp_buf[rsp_count].msg = RSP_NACK;
                }
            }
            // is that possible for a REQ_WB in SPX_S???
            else if (data.state == SPX_S)
            {
                // having blocking states;
                tu_req.u_state = LLC_SV;
                // wait();
                rsp_buf[rsp_count].msg = FWD_INV;
                InvSharers(llc_data.sharers, SPX, rsp_buf[rsp_count].dest);
            }
            break;
        }
        }
    }
}

void LLC::rcv_req_word(id_t &tu_id, MSG &tu_req, int rsp_count)
// Behaviour when LLC receives an external request from TU (Table III).
{
    breakdown(llc_addr, tu_req.addr);
    fetch_line(llc_addr, llc_data);
    // msg_init();

    // if (tu_req.mask.any())
    // {
    //     WordExt(data, llc_data, tu_req.mask);
    // }
    // else

    rcv_req(tu_id, tu_req, rsp_count, tu_req.mask, llc_data);
}

void LLC::rcv_req_line(id_t &tu_id, MSG &tu_req, int rsp_count)
// LLC is always word granularity; if receive a line granularity request, breakdown into word granularity;
{
    breakdown(llc_addr, tu_req.addr);
    fetch_line(llc_addr, llc_data);
    // msg_init();

    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        rcv_req(tu_id, tu_req, rsp_count, bitset<WORDS_OFF>(i), llc_data);
    }
}

void LLC::snd_req() {}

void LLC::snd_rsp() {}