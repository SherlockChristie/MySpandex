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

bool LineReady(state_t &state)
// For TU to decide whether the whole line is ready or not. (line_state is valid, and no word is in Owned.)
{
    std::bitset<STATE_LINE> state_line = BitSub<STATE_BITS, STATE_LINE>(state, STATE_WORDS);
    std::bitset<STATE_WORDS> state_words = BitSub<STATE_BITS, STATE_WORDS>(state, 0);

    if ((state_line == SPX_V) && (state_words.none()))
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

void WordIns(word_t word, word_offset_t offset, line_t line)
// Insert a word in the line according to the offset.
{
    uint8_t byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        line[byte_off] = word[i];
        byte_off++;
    }
}

void WordExt(word_t word, word_offset_t offset, line_t line)
// Extract a word in the line according to the offset.
{
    uint8_t byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        word[i] = line[byte_off];
        byte_off++;
    }
}

// void MsgInit()
// {
// }

#endif // BIT_UTILS_HPP