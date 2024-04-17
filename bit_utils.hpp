#ifndef BIT_UTILS_HPP
#define BIT_UTILS_HPP

#include <iostream>
#include <bitset>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cstdint>
// using namespace std;
#include "consts.hpp"
#include "blocks.hpp"

void wait(){};

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

bool LineReady(llc_state_t &state)
// For TU to decide whether the whole line is ready or not. (line_state is valid, and no word is in Owned.)
{
    std::bitset<STATE_NUM> state_line = BitSub<STATE_BITS, STATE_NUM>(state, STATE_WORDS);
    std::bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(state, 0);

    if ((state_line == LLC_V) && (state_words.none()))
        return 1;
    else
        return 0;
}

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
//     uint8_t byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
//     for (int i = 0; i < BYTES_PER_WORD; i++)
//     {
//         line[byte_off] = word[i];
//         byte_off++;
//     }
// }

// void WordExt(word_t word, word_offset_t offset, line_t line)
// // Extract a word from the line according to the offset.
// {
//     uint8_t byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
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
void WordIns(DATA_WORD &word, DATA_LINE &line, word_offset_t &offset)
// Insert a word and its state in the line according to the offset.
{
    unsigned long off = offset.to_ulong();
    unsigned long byte_off = off * BYTES_PER_WORD;
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
        line.state.set(off);
}

void WordExt(DATA_WORD &word, DATA_LINE &line, word_offset_t &offset)
// Extract a word and its state from the line according to the offset.
{
    unsigned long off = offset.to_ulong();
    unsigned long byte_off = off * BYTES_PER_WORD;
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        word.data[i] = line.data[byte_off];
        byte_off++;
    }
    if (line.state.test(off))
        word.state = SPX_O;
    else
    {
        std::bitset<STATE_NUM> temp = BitSub<STATE_BITS, STATE_NUM>(line.state, STATE_WORDS);
        std::bitset<STATE_DEV - STATE_NUM> zero(0);
        word.state = BitCat(zero, temp);
    }
}

id_t find_owner(DATA_LINE &data_line)
{
    DATA_WORD owner_word;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (data_line.state.test(i))
        {
            std::bitset<WORDS_OFF> off(i);
            WordExt(owner_word, data_line, off);
            break; // not worried about multiple owners since it is exclusive.
        }
    }
    // the data field itself stores the owner id;
    id_t owner_bits(owner_word.data[0]);
    // id_t only has 2 bits; so just get owner_word.data[0];
    return owner_bits;
}

id_t InvSharers(sharers_t sharers, id_t self, id_t dest)
// Invalidate sharers for a downgrade from state S.
{
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

#endif // BIT_UTILS_HPP