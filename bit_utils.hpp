#ifndef BIT_UTILS_HPP
#define BIT_UTILS_HPP

#include <iostream>
#include <bitset>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
#include <vector>
// using namespace std;
#include "blocks.hpp"
#include "classes.hpp"
#include "consts.hpp"

// void wait() {}

constexpr std::size_t ULONGLONG_BITS = std::numeric_limits<unsigned long long>::digits;
// Usage: new = BitCat[<a_len,b_len>](a,b);
// Meaning: Concatenate 2 bitsets(smaller than ULONGLONG_BITS);
template <std::size_t N1, std::size_t N2>
typename std::enable_if<(N1 + N2) <= ULONGLONG_BITS, std::bitset<N1 + N2>>::type // efficient for small sizes
BitCat(const std::bitset<N1> &a, const std::bitset<N2> &b)
{
    return (a.to_ullong() << N2) | b.to_ullong();
}

// Usage: the_sub_bitset = BitSub<len_big,len_sub>(the_big_bitset,offset);
// Meaning: the_sub_bitset = the_big_bitset[from (offset) to (offset + len_sub)];
// TODO: <N1,N2> not optional, not consistent with BitCat.
template <std::size_t N1, std::size_t N2>
std::bitset<N2> BitSub(std::bitset<N1> &b, std::size_t offset)
{
    return std::bitset<N2>((b >> offset).to_ullong());
}

void LineCopy(line_t &dest, const line_t &src)
{
    std::memcpy(dest, src, sizeof(line_t));
}

// bool LineReady(state_t &state)
// // For TU to decide whether the whole line is ready or not. (line_state is valid, and no word is in Owned.)
// {
//     std::bitset<STATE_LINE> state_line = BitSub<STATE_BITS, STATE_LINE>(state, STATE_WORDS);
//     std::bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(state, 0);

//     if ((state_line == LLC_V) && (state_words.none()))
//         return 1;
//     else
//         return 0;
// }

// void WordIns(word_t word, word_offset_t offset, line_t *line)
// {
//     byte_t *byte_off = (byte_t *)line;
//     byte_off += (offset.to_ulong()) * BYTES_PER_WORD;
//     for(int i = 0; i <BYTES_PER_WORD;i++)
//     {
//         *byte_off++ = *word++;
//     }
// }

// Should insert the state as well.
// void WordIns(word_t word, word_offset_t offset, line_t line)
// // Insert a word in the line according to the offset.
// {
//     int byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
//     for (int i = 0; i < BYTES_PER_WORD; i++)
//     {
//         line[byte_off] = word[i];
//         byte_off++;
//     }
// }

// void WordExt(word_t word, word_offset_t offset, line_t line)
// // Extract a word from the line according to the offset.
// {
//     int byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
//     for (int i = 0; i < BYTES_PER_WORD; i++)
//     {
//         word[i] = line[byte_off];
//         byte_off++;
//     }
// }

// bool is_word_owned(DATA_LINE &line)
// {
//     std::bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(line.state, 0);
//     return state_words.any();
// }

// void WordInsState(DATA_WORD word, word_offset_t mask, DATA_LINE line)
// // Insert a word's state in the line's state according to the offset.
// {
//     WordExt(word.data, mask, line.data);
//     std::bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(line.state, 0);
//     for (int i = 0; i < WORDS_PER_LINE; i++)
//     {
//         if (line.state.test(i))
//             WordExt(word.data, i, line.data);
//     }
// }

// void WordExtState(word_t word, word_offset_t mask, line_t line)
// // Extract a word's state from the line's state according to the offset.
// {
// }

// WordIns and WordExt can only be used in LLC and TU.
// the word's state in the DEV relies on state_mapping_back();
void WordIns(DATA_WORD &word, DATA_LINE &line, unsigned long offset)
// Insert a word and its state in the line according to the offset.
{
    unsigned long byte_off = offset * BYTES_PER_WORD;
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        line.data[byte_off] = word.data[i];
        byte_off++;
    }
    // only when word is in O state it would affect the line's state;
    if (word.state == SPX_O)
        // if (word.state == DEV_O || word.state == LLC_O)
        // won't have problem since both DEV_O and LLC_O are 2;
        // bool is_dev not needed;
        line.line_state.set(offset);
    else
        line.line_state = word.state;
    // 行状态等同于字状态;
}

void WordExt(DATA_WORD &word, DATA_LINE &line, unsigned long offset)
// Extract a word and its state from the line according to the offset.
{
    unsigned long byte_off = offset * BYTES_PER_WORD;
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        word.data[i] = line.data[byte_off];
        byte_off++;
    }
    if (line.line_state.test(offset))
        word.state = SPX_O;
    else
    {
        word.state = line.line_state;
    }
}

id_num_t FindOwner(DATA_LINE &data)
{
    DATA_WORD owner_word;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (data.word_state.test(i))
        {
            WordExt(owner_word, data, i);
            break; // not worried about multiple owners since it is exclusive.
        }
    }
    // the data field itself stores the owner id;
    id_num_t owner_bits(owner_word.data[0]);
    // id_num_t only has 2 bits; so just get owner_word.data[0];
    return owner_bits;
}

id_bit_t InvSharers(id_bit_t sharers, id_bit_t self)
// Invalidate sharers for a downgrade from state S.
{
    id_bit_t dest;
    dest = sharers;
    dest &= (~self);
    // Send message to all sharers except itself.
    // for (int i = 0; i < MAX_DEVS; i++)
    // {
    //     if (i == self)
    //         continue;
    //     if (sharers.test(i))
    //     {
    //         message.dest.set(i);
    //     }
    // }
    return dest;
}

// void MsgInit()
// {
// }

bool is_conflict(std::vector<MSG> &req_buf, MSG &new_msg)
{
    if (!req_buf.empty())
    {
        for (int i = 0; i < req_buf.size(); i++)
        {
            if (new_msg.addr == req_buf[i].addr)
                return true;
        }
    }
    return false;
}

void rcv_rsp_single(MSG &rsp_in, unsigned long offset, DATA_LINE &data_line)
{
    DATA_WORD data;
    WordExt(data, data_line, offset);

    switch (rsp_in.msg)
    {
    case RSP_V:
    {
        data.state = SPX_V;
        break;
    }
    case RSP_S:
    {
        data.state = SPX_S;
        break;
    }
    case RSP_WTdata:
    {
        data.state = SPX_I;
        break;
    }
    case RSP_Odata:
    {
        data.state = SPX_O;
        break;
    }
    }

    WordIns(data, data_line, offset);
}

void get_rsp(MSG rsp, std::vector<MSG> &req_buf, id_num_t id)
// get a rsp, find match in req_buf;
{
    MSG gen = rsp;
    int req_len = req_buf.size();
    for (int i = 0; i < req_len; i++)
    {
        if (rsp.id == req_buf[i].id)
        {
            if (rsp.gran == GRAN_LINE)
            { // 同时释放req和rsp;
                // 但是rsp已经在总线释放了？
                req_buf.erase(req_buf.begin() + i);
            }
            else
            {
                req_buf[i].ok_mask |= rsp.mask;
                if (req_buf[i].ok_mask.all()) // 收集到了所有rsp;
                {
                    gen.gran = GRAN_LINE;
                    req_buf.erase(req_buf.begin() + i);
                    get_rsp(gen, devs[id.to_ulong()].req_buf, id);
                    // 无需释放gen, 因为 DEV 和 TU 的通信不经过总线;
                }
            }
        }
    }
}

void get_msg()
{
    MSG tmp;
    tmp = bus.front();
    // for (int i = 0; i < MAX_DEVS; i++)
    {
        if (tmp.msg < RSP_S) // req or fwd
        {
            if (tmp.dest.test(0))
                llc.req_buf.push_back(tmp);
            if (tmp.dest.test(1))
                tus[CPU].req_buf.push_back(tmp);
            if (tmp.dest.test(2))
                tus[GPU].req_buf.push_back(tmp);
            if (tmp.dest.test(3))
                tus[ACC].req_buf.push_back(tmp);
        }
        else // rsp;
        {
            // if (tmp.dest.test(0))
            //     llc.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(1))
            //     tcpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(2))
            //     tgpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(3))
            //     tacc.rsp_buf.push_back(tmp);

            // 无需将rsp_in入队，直接对对碰消掉req和rsp_in;
            if (tmp.dest.test(0))
                get_rsp(tmp, llc.req_buf, SPX);
            if (tmp.dest.test(1))
                get_rsp(tmp, tus[CPU].req_buf, CPU);
            if (tmp.dest.test(2))
                get_rsp(tmp, tus[GPU].req_buf, GPU);
            if (tmp.dest.test(3))
                get_rsp(tmp, tus[ACC].req_buf, ACC);
        }
    }
    bus.erase(bus.begin());
}

// void put_req(std::vector<MSG> &req)
// // Put all the req_buf to the bus.
// {
//     int req_len = req.size();
//     for (int i = 0; i < req_len; i++)
//     // while (!req.empty())
//     {
//         MSG tmp;
//         tmp = req.front();
//         bus.push_back(tmp);
//         // all req stays at the vector until its rsp in;
//     }
// }
void put_rsp(std::vector<MSG> &rsp)
// Put all the rsp_buf to the bus.
{
    int rsp_len = rsp.size();
    for (int i = 0; i < rsp_len; i++)
    // while (!rsp.empty())
    {
        MSG tmp;
        tmp = rsp.front();
        bus.push_back(tmp);
        // if (tmp.msg < FWD_REQ_S) // rsp, not fwd;
        // {
        rsp.erase(rsp.begin());
        //}
    }
}

// void get_req(std::vector<MSG> &req)
// // get a req, insert it in req_buf;
// {
//     req.push_back(bus.front()); // req_buf入队;
//     bus.erase(bus.begin());     // bus出队;
// }

// bool is_single(MSG &msg, DATA_LINE &line)
// {
//     if (msg.gran == GRAN_LINE)
//         return 1;
//     else
//     {
//         if (msg.mask.all())
//         {
//             if (line.word_state.none() || line.word_state.all())
//                 return 1;
//         }
//     }
//     return 0;
// }

void MsgCoalesce(std::vector<MSG> &buf)
{
    int len = buf.size();
    for (int i = 0; i < len - 1; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            if ((buf[i].id == buf[j].id) && (buf[i].dest == buf[j].dest) && (buf[i].addr == buf[j].addr) && (buf[i].msg == buf[j].msg))
            {
                buf[i].mask |= buf[j].mask;
                buf.erase(buf.begin() + j);
            }
        }
    }
}
#endif // BIT_UTILS_HPP