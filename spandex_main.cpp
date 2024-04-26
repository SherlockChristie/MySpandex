#include "bit_utils.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "consts.hpp"
// #include <vector>
using namespace std;

// std::vector<MSG> bus;
// DEV cpu, gpu, acc;
// TU tcpu, tgpu, tacc;
// LLC llc;

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
    
    DEV *p = devs;
    p+= ACC;
    devs[ACC].req_buf.push_back(fig_a);
    tus[ACC].mapping_wrapper(p);
    devs[ACC].req_buf.front().Display();

    return 0;
}