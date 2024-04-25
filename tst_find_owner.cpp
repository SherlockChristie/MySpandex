#include "bit_utils.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "consts.hpp"
using namespace std;

id_bit_t FindOwner(DATA_LINE &data_line)
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
    for (int i = BYTES_PER_WORD - 1; i >= 0; i--)
    {
        printf("%d ", owner_word.data[i]);
    }
    printf("\n");

    // std::bitset<BITS_PER_BYTE*BYTES_PER_WORD> id;

    // the data field itself stores the owner id;
    id_bit_t owner_bits(owner_word.data[0]);
    // is that the address or the data? may have problems.
    return owner_bits;
}

int main()
{
    DATA_LINE tst;
    spx_line_state_t states("010100"); // 01_0100; 01:Line in Valid; 0100: Word2 in O
    // tst.data[DEV_COL] = {0};
    tst.state = states;
    tst.sharers = {0};
    for (int i = 0; i < DEV_COL; i++)
    {
        tst.data[i] = 0;
    }
    tst.data[2 * BYTES_PER_WORD] = 3; // owner_id =3;
    // Little-edian;
    for (int i = DEV_COL - 1; i >= 0; i--)
    {
        // cout << tst.data[i] << " ";
        printf("%d ", tst.data[i]);
    }
    cout << endl;
    cout << tst.state << endl;
    cout << tst.sharers << endl;

    id_bit_t owner_id = FindOwner(tst);
    unsigned long id = owner_id.to_ulong();
    printf("%d\n", id);
    return 0;
}