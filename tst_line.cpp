#include <cstdint> // For int
#include <cstring> // For std::memcpy
#include <iostream> // For std::cout

#define BYTES_PER_WORD 4
#define WORDS_PER_LINE 4

typedef uint8_t byte_t;
typedef byte_t word_t[BYTES_PER_WORD]; // Defines a word of 4 bytes
typedef word_t line_t[WORDS_PER_LINE]; // Defines a line of 4 words

// Function to copy one line to another
void copyLine(line_t& dest, line_t& src) {
    std::memcpy(dest, src, sizeof(line_t));
}

int main() {
    line_t line_a, line_b;

    // Example: Initialize line_b with some values
    for (int i = 0; i < WORDS_PER_LINE; ++i) {
        for (int j = 0; j < BYTES_PER_WORD; ++j) {
            line_b[i][j] = i * BYTES_PER_WORD + j; // Just an example initialization
        }
    }

    // Copy line_b to line_a
    copyLine(line_a, line_b);

    // Print line_a to verify the copy
    for (int i = 0; i < WORDS_PER_LINE; ++i) {
        for (int j = 0; j < BYTES_PER_WORD; ++j) {
            std::cout << static_cast<int>(line_a[i][j]) << ' ';
        }
        std::cout << '\n';
    }

    return 0;
}
