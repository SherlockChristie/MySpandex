#include <vector>
#include <iostream>
#include "classes.hpp"

int main()
{
    std::vector<MSG> bus;
    MSG msg;
    MSG tst;
    msg.dest = std::bitset<MAX_DEVS> (2);
    msg.addr = std::bitset<32> (0x66ccff);
    msg.msg = REQ_Odata;
    msg.gran = GRAN_LINE;
    msg.u_state = LLC_I;

    bus.push(msg);
    tst = bus.front();
    std::cout<<tst.addr<<std::endl;
    return 0;
}