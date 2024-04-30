#ifndef BIT_UTILS_HPP
#define BIT_UTILS_HPP

// using namespace std;
#include "headers.hpp"
#include "blocks.hpp"
#include "consts.hpp"

// void wait() {}

// constexpr std::size_t ULONGLONG_BITS = std::numeric_limits<unsigned long long>::digits;
// Usage: new = BitCat[<a_len,b_len>](a,b);
// Meaning: Concatenate 2 bitsets(smaller than ULONGLONG_BITS);
// template <std::size_t N1, std::size_t N2>
// typename std::enable_if<(N1 + N2) <= ULONGLONG_BITS, std::bitset<N1 + N2>>::type // efficient for small sizes
// BitCat(const std::bitset<N1> &a, const std::bitset<N2> &b)
// {
//     return (a.to_ullong() << N2) | b.to_ullong();
// }

template <std::size_t N1, std::size_t N2>
std::bitset<N1 + N2> BitCat(const std::bitset<N1> &a, const std::bitset<N2> &b)
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

void LineCopy(line_t &dest, const line_t &src);

// WordIns and WordExt can only be used in LLC and TU.
// the word's state in the DEV relies on data_mapping_back();
void WordIns(DATA_WORD &word, DATA_LINE &line, unsigned long offset);
// Insert a word and its state in the line according to the offset.

void WordExt(DATA_WORD &word, DATA_LINE &line, unsigned long offset);
// Extract a word and its state from the line according to the offset.

id_num_t FindOwner(DATA_LINE &data);

id_bit_t InvSharers(id_bit_t sharers, id_bit_t self);

#endif // BIT_UTILS_HPP