#include <iostream>  
#include <bitset> // 包含 bitset 头文件  
#include <type_traits>
#include <limits>
#define BYTES_PER_WORD 8

constexpr std::size_t ULONGLONG_BITS = std::numeric_limits<unsigned long long>::digits;
// Usage: new = BitCat(a,b);
// Meaning: Concatenate 2 bitsets(smaller than ULONGLONG_BITS);
template <std::size_t N1, std::size_t N2>
typename std::enable_if<(N1 + N2) <= ULONGLONG_BITS, std::bitset<N1 + N2>>::type // efficient for small sizes
BitCat(const std::bitset<N1> &a, const std::bitset<N2> &b)
{
    return (a.to_ullong() << N2) | b.to_ullong();
}

template<std::size_t N, std::size_t len>
std::bitset<len> getSubBitset(std::bitset<N> &b, std::size_t start)
{
    return std::bitset<len>((b >> start).to_ulong());
}
 
int main(){
    std::bitset<8> b(0x35);
    std::bitset<10> a(0x42);
    std::cout << b << std::endl;
    ///截取bit2~bit6，长度为5
    std::cout << getSubBitset<BYTES_PER_WORD, 5>(b, 2) << std::endl;
    ///截取bit0~bit3，长度为4
    std::cout << BitCat<10,8>(a,b) << std::endl;
    std::cout << getSubBitset<8, 4>(b, 0) << std::endl;
    ///截取bit4~bit7，长度为4
    std::cout << getSubBitset<8, 4>(b, 4) << std::endl;
    return 0;
}
