#include "headers.hpp"
#include "consts.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "bit_utils.hpp"
#include "msg_utils.hpp"
// #include <vector>
using namespace std;

// 将下面几个设置为工程全局变量;
std::vector<MSG> bus;
DEV devs[MAX_DEVS];
TU tus[MAX_DEVS];
LLC llc;

void case_init()
{
    // Fig 1(a);

    line_t data_CF = {0x00, 0x66, 0xCF, 0xCF, 0x01, 0x66, 0xCF, 0xCF, 0x10, 0x66, 0xCF, 0xCF, 0x11, 0x66, 0xCF, 0xCF};
    LineCopy(llc.cache[0xCF], data_CF);
    llc.line_state_buf[0xCF] = LLC_V;
    llc.tag_buf[0xCF] = 0x19B; // 0b 0110_0110_11 -> 01_1001_1011
}

int main()
{
    // init;
    // reset;

    case_init();
    // line_t data_CF = {0x00, 0x66, 0xCF, 0xCF, 0x01, 0x66, 0xCF, 0xCF, 0x10, 0x66, 0xCF, 0xCF, 0x11, 0x66, 0xCF, 0xCF};
    // LineCopy(llc.cache[0xCF], data_CF);
    for (int i = BYTES_PER_WORD * WORDS_PER_LINE - 1; i >= 0; i--)
    {
        printf("%x ", llc.cache[0xCF][i]);
    }
    // cout << endl;

    MSG fig_a;
    fig_a.id = 1;
    fig_a.mask = 0xA; // 0b1010
    // no need for a DEV req to determine dest; always LLC; determined in tu.req_mapping();
    fig_a.addr = 0x66CCFF;
    fig_a.msg = WRITE;
    // gran/ok_mask also determined in tu.req_mapping();
    // u_state/retry_times determined in ?;
    // no data_line/data_word needed;

    fig_a.msg_display();

    devs[ACC].req_buf.push_back(fig_a);
    tus[ACC].req_mapping(ACC, fig_a);

    MSG mapped = tus[ACC].req_buf.front();
    mapped.msg_display();
    // bus.front().msg_display();
    get_msg();
    // llc.req_buf.front().msg_display();
    llc.rcv_req(bitset<MAX_DEVS_BITS>(ACC), mapped);

    bus.front().msg_display();

    cout << "ok" << endl;
    // cout<<llc.word_state_buf[0xCF]<< endl;
    return 0;
}