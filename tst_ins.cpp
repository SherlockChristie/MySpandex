#include <iostream>
#include <bitset>
#include "classes.hpp"

void WordIns(word_t word, word_offset_t offset, line_t &data)
{
    // Calculate the bit offset based on the offset.
    // Since offset represents an index, we multiply by the size of a word in bits.
    line_t bitOffset = (offset << WORDS_OFF) << BYTES_OFF;

    // Create a offset to clear the section of data where the word will be inserted.
    word_t clearoffset = ~(word_t(0)); // This creates a offset of all 1's the size of a word.

    // Clear the section where the word will be inserted.
    data &= ( clearoffset<< bitOffset);

    // Insert the word into the cleared section.
    data |= (word << bitOffset);
}

int main()
{
    // Example usage:
    word_t word(0b11110000111100001111000011110000); // Example word.
    word_offset_t offset(0b10);                      // Target position in the line.
    line_t data(0);                                  // Initially empty data.

    WordIns(word, offset, data);

    // Print the result.
    std::cout << data << std::endl;

    return 0;
}
