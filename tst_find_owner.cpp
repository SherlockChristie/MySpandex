#include "bit_utils.hpp"
#include "consts.hpp"

id_t find_owner(DATA_LINE &data_line)
{
    word_t owner_word;
    for (int i = 0; i < WORDS_PER_LINE; i++)
    {
        if (data_line.state.test(i))
        {
            WordExt(owner_word, i, data_line.data);
            break; // not worried about multiple owners since it is exclusive.
        }
    }
    // the data field itself stores the owner id;
    id_t owner_bits(owner_word);
    // is that the address or the data? may have problems.
    return owner_bits;
}

int main()
{
    DATA_LINE tst;
    tst.data[DEV_COL] = {0};
    tst.state("010100");
    tst.sharers = {0};

    return 0;
}