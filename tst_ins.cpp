#include <iostream>
#include <bitset>
#include "cache_block.hpp"

// not a good function.
// void WordIns(word_t word, word_offset_t offset, line_t &data)
// {
//     // Calculate the bit offset based on the offset.
//     // Since offset represents an index, we multiply by the size of a word in bits.
//     line_t bitOffset = (offset << WORDS_OFF) << BYTES_OFF;

//     // Create a offset to clear the section of data where the word will be inserted.
//     word_t clearoffset = ~(word_t(0)); // This creates a offset of all 1's the size of a word.

//     // Clear the section where the word will be inserted.
//     data &= ( clearoffset<< bitOffset);

//     // Insert the word into the cleared section.
//     data |= (word << bitOffset);
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

void WordIns(word_t word, word_offset_t offset, line_t line)
{
    int byte_off = (offset.to_ulong()) * BYTES_PER_WORD;
    printf("%d\n", byte_off);
    for (int i = 0; i < BYTES_PER_WORD; i++)
    {
        line[byte_off] = word[i];
        byte_off++;
    }
}

int main()
{
    // Example usage:
    word_t word = {0b11110000, 0b11110000, 0b11110000, 0b11110000}; // Example word.
    word_offset_t offset(0b10);                                     // Target position in the line.
    line_t data = {0};                                              // Initially empty data.

    WordIns(word, offset, data);

    // std::bitset<128> binary(data);
    // Print the result.
    for (int i = 0; i < 4; i++)
    {
        printf("%x ", word[i]);
    }
    printf("\n");
    // for (int i = 0; i < 16; i++)
    for (int i = 15; i >=0; i--)
    // Little endian!!!!!
    {
        printf("%x ", data[i]);
    }

    return 0;
}
