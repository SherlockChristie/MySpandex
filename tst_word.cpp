#include <algorithm> // For std::copy
#include <cstdint>   // For uint8_t
#include <iostream>

#define BYTES_PER_WORD 4
#define WORDS_PER_LINE 4

typedef uint8_t byte_t;
typedef byte_t word_t[BYTES_PER_WORD];
typedef word_t line_t[WORDS_PER_LINE];

word_t data_line[WORDS_PER_LINE];
word_t word3, word2, word1, word0;

// Function to initialize example data in wordi
void initializeWords() {
    // Example initialization
    for (int i = 0; i < BYTES_PER_WORD; ++i) {
        word3[i] = 3;
        word2[i] = 2;
        word1[i] = 1;
        word0[i] = 0;
    }
}

void assignWordToDataLine(int i, const word_t& word) {
    if (i >= 0 && i < WORDS_PER_LINE) {
        std::copy(word, word + BYTES_PER_WORD, data_line[i]);
    }
}

int main() {
    initializeWords();

    // Assign each word to data_line
    assignWordToDataLine(0, word0);
    assignWordToDataLine(1, word1);
    assignWordToDataLine(2, word2);
    assignWordToDataLine(3, word3);

    // Optional: Print to verify
    for (int i = 0; i < WORDS_PER_LINE; ++i) {
        for (int j = 0; j < BYTES_PER_WORD; ++j) {
            std::cout << static_cast<int>(data_line[i][j]) << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
