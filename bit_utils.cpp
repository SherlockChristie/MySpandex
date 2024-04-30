#include "bit_utils.hpp"

void LineCopy(line_t &dest, const line_t &src)
// void LineCopy(line_t *dest, const line_t &src)
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
        line.word_state.set(offset);
    else
    {
        line.line_state = word.state; // 行状态等同于字状态;
        line.word_state.reset(offset); // 对于从 O 转到其他状态的字，应当更新行状态;
    }
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
    // if (line.line_state.test(offset))
    if (line.word_state.test(offset))
        // word_state!!!!! not line_state!!!!!
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
