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
}

int main()
{
    // init;
    // reset;

    MSG fig_a;
    fig_a.id = 1;
    fig_a.mask = 0xA; // 0b1010
    // no need for a DEV req to determine dest; always LLC; determined in tu.req_mapping();
    fig_a.addr = 0x66CCFF;
    fig_a.msg = WRITE;
    // gran/ok_mask also determined in tu.req_mapping();
    // u_state/retry_times determined in ?;
    // no data_line/data_word needed;

    fig_a.Display();

    devs[ACC].req_buf.push_back(fig_a);
    // tus[ACC].tst();
    tus[ACC].req_mapping(ACC,fig_a);
    // Yeah!!!!!!!!!!!!!!!!!!
    // tus[ACC].mapping_wrapper(devs[ACC]);
    // tus[ACC].mapping_wrapper(*(devs[ACC]));
    // std::cout << "after" << std::endl;
    // devs[ACC].req_buf.front().Display();

    return 0;
}