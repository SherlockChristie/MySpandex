#include "headers.hpp"
#include "consts.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "bit_utils.hpp"
#include "msg_utils.hpp"
using namespace std;

// TODO: 用 fetch_line() 和 back_line() 函数初始化而不是手动;
// TODO: 现在 LLC 的 wait_time是统一的，但实际上应该不是;
// TODO: 用 Makefile 编译文件而非 run code;
// 有时候会出现奇怪的无法运行 bug: tempCodeRunnerFile.cpp:1:13: error: expected constructor, destructor, or type conversion before '(' token
// link funs_xxx.cpp(3个) to classes.hpp(1个);
// TODO: 负载发出的字粒度请求应当是纯“字粒度”的（通过word_offset标识），然后对于同一行的同类型字粒度请求，由 Spandex 合并为一个 multi-word req;

// 将下面几个设置为工程全局变量;
std::vector<MSG> bus;
DEV devs[MAX_DEVS];
TU tus[MAX_DEVS];
LLC llc;

MSG Req_001, Req_002, Req_003, Req_004;

// TODO: 也清除内存;
void reset()
{
    for (int i = 0; i < MAX_DEVS; i++)
    {
        devs[i].req_buf.clear();
        devs[i].rsp_buf.clear();
        tus[i].req_buf.clear();
        tus[i].rsp_buf.clear();
    }
    llc.req_buf.clear();
    llc.rsp_buf.clear();
    bus.clear();
}

void init()
{
    for (int i = 0; i < MAX_DEVS; i++)
    {
        devs[i].dev_id = i;
        tus[i].tu_id = i;
    }
}

void init_a()
{
    // ------------- Fig 1(a) --------------
    // Handling word granularity ReqO and ReqWT. (Avoiding false sharing)
    // addr: 0x66CCFF = 0b 0110 0110 11/00 1100 1111 /11/11

    // Data init;
    // LLC init;
    line_t data_CF = {0x00, 0x66, 0xCC, 0xFF, 0x01, 0x66, 0xCC, 0xFF, 0x10, 0x66, 0xCC, 0xFF, 0x11, 0x66, 0xCC, 0xFF};
    LineCopy(llc.cache[0xCF], data_CF);
    llc.line_state_buf[0xCF] = LLC_V;
    llc.tag_buf[0xCF] = 0x19B; // 0b 0110_0110_11 -> 01_1001_1011

    // addr: 0x66CCFF = 0b 0110 0110 1100 11/00 1111 /11/11
    // GPU init (V in Fig 1(a), I in Fig 1(c); but not necessary);
    // ACC init (Used in Fig 1(c));
    line_t data_ACC_0F = {0x00, 0x66, 0xCC, 0xFF, 0x01, 0x01, 0xAC, 0xCF, 0x00, 0x66, 0xCC, 0xFF, 0x11, 0x11, 0xAC, 0xCF};
    // word[1] & word[3] Owned, diff from data_CF, trigger ReqO;
    LineCopy(devs[ACC].cache[0xF], data_ACC_0F);
    devs[ACC].state_buf[0xF] = DEV_O;
    devs[ACC].tag_buf[0xF] = 0x19B3; // 0b 0110 0110 1100 11 -> 01 1001 1011 0011

    // Req init;
    // ReqO;
    Req_001.id = 1;
    Req_001.mask = 0xA; // 0b1010
    // no need for a DEV req to determine dest; always LLC; determined in tu.req_mapping();
    Req_001.addr = 0x66CCFF;
    Req_001.msg = WRITE;
    Req_001.time_sp = TIME_L1;
    Req_001.time_hm = TIME_L1;
    // gran/ok_mask also determined in tu.req_mapping();
    // u_state/retry_times determined in ?;
    // no data_line/data_word needed;
    // Req_001.msg_display();

    // ReqWT;
    Req_002.id = 2;
    Req_002.mask = 0x5; // 0b0101
    Req_002.addr = 0x66CCFF;
    Req_002.msg = WRITE;
    Req_002.time_sp = TIME_L1;
    Req_002.time_hm = TIME_L1;
    // Req_002.msg_display();
    // ------------------------------------
}

void do_a()
{
    init_a();
    int step = 0;
    for (step = 1; step < 3; step++)
    {
        cout << "Steps: " << step << endl;
        if (step == 1)
        {
            devs[ACC].req_buf.push_back(Req_001);
            tus[ACC].mapping_wrapper(devs[ACC]);
        }
        else if (step == 2)
        {
            devs[GPU].req_buf.push_back(Req_002);
            tus[GPU].mapping_wrapper(devs[GPU]);
        }
        get_msg(); // 上升沿get_msg();
        // llc.req_buf.front().msg_display();
        // llc.rcv_req(bitset<MAX_DEVS_BITS>(ACC), mapped);
        llc.rcv_req();
        get_msg(); // 下降沿get_msg();
    }
}

void init_b()
{
    // ------------- Fig 1(b) --------------
    // Handling word granularity ReqWT+data for remotely owned data.
    // Wish see GPU's data, not ACC's data after the write serialization.
    // addr: 0x39C5BB = 0b 0011 1001 11/00 0101 1011 /10/11

    // Data init;
    // LLC init;
    line_t data_5B = {ACC, 0x00, 0x00, 0x00, ACC, 0x00, 0x00, 0x00, ACC, 0x00, 0x00, 0x00, ACC, 0x00, 0x00, 0x00};
    // Storing owners' id.
    LineCopy(llc.cache[0x5B], data_5B);
    llc.line_state_buf[0x5B] = LLC_V;
    llc.word_state_buf[0x5B].set(); // set all to O state;
    llc.tag_buf[0x5B] = 0xE7;       // 0b 0011_1001_11 -> 00_1110_0111

    // addr: 0x39C5BB = 0b 0011 1001 1100 01/01 1011 /10/11
    // ACC init;
    line_t data_ACC_1B = {0x00, 0x39, 0xC5, 0xBB, 0x01, 0x39, 0xC5, 0xBB, 0x10, 0x39, 0xC5, 0xBB, 0x11, 0x39, 0xC5, 0xBB};
    LineCopy(devs[ACC].cache[0x1B], data_ACC_1B);
    devs[ACC].state_buf[0x1B] = DEV_O;
    devs[ACC].tag_buf[0x1B] = 0xE71; // 0b 0011_1001_1100_01 -> 00_1110_0111_0001
    // GPU init;
    line_t data_GPU_1B = {0x00, 0x00, 0x0E, 0x71, 0x01, 0x00, 0x0E, 0x71, 0x10, 0x00, 0x0E, 0x71, 0x11, 0x00, 0x0E, 0x71};
    LineCopy(devs[GPU].cache[0x1B], data_GPU_1B);
    devs[GPU].state_buf[0x1B] = DEV_V;
    // should be I??? a store miss??? but assume Valid;
    // Yes it's Owned in ACC; but just assume it is the data to be stored;
    devs[GPU].tag_buf[0x1B] = 0xE71;

    // Req init;
    // ReqWTdata;
    Req_003.id = 3;
    Req_003.mask = 0x5; // 0b0101
    // no need for a DEV req to determine dest; always LLC; determined in tu.req_mapping();
    Req_003.addr = 0x39C5BB;
    Req_003.msg = RMW;
    Req_003.time_sp = TIME_L1;
    Req_003.time_hm = TIME_L1;
    // Req_003.msg_display();
    // ------------------------------------
}

void do_b()
{
    init_b();
    int step = 0;
    for (step = 1; step < 3; step++)
    {
        cout << "Steps: " << step << endl;
        if (step == 1)
        {
            devs[GPU].req_buf.push_back(Req_003);
            tus[GPU].mapping_wrapper(devs[GPU]);
        }
        get_msg();
        // buf_detailed(bus);
        // buf_detailed(llc.req_buf);
        // buf_detailed(llc.rsp_buf);
        llc.rcv_req();
        get_msg();
        if (step == 1)
        {
            // buf_detailed(bus);
            // buf_detailed(llc.req_buf);
            // buf_detailed(llc.rsp_buf);
            tus[ACC].rcv_fwd();
            // buf_detailed(bus);
            // buf_detailed(llc.req_buf);
            // buf_detailed(llc.rsp_buf);
        }

        // cout << "------ LLC TEST -------" << endl;
        // int len = llc.req_buf.size();
        // for (int i = 0; i < len; i++)
        // {
        //     llc.req_buf[i].msg_display();
        // }
    }
}

// Must do a first if you want to go through c;
void init_c()
{
    // ------------- Fig 1(c) --------------
    // Handling line granularity ReqV.
    // Using 0x66CCFF in init_a();

    // Req init;
    // ReqV;
    Req_004.id = 4;
    Req_004.addr = 0x66CCFF;
    Req_004.msg = READ;
    Req_004.time_sp = TIME_L1;
    Req_004.time_hm = TIME_L1;
    // Req_004.msg_display();
    // ------------------------------------
}

// TODO: ACC 的 O 状态应该是字粒度的？现在设置成了行粒度;
void do_c()
{
    do_a();

    cout << "------------------------- do_a() above to get OVOV line ----------------------" << endl;
    cout << "--------------------------------- do_c() below -------------------------------" << endl;

    init_c();
    int step = 0;
    for (step = 1; step < 3; step++)
    {
        cout << "Steps: " << step << endl;
        if (step == 1)
        {
            devs[GPU].req_buf.push_back(Req_004);
            tus[GPU].mapping_wrapper(devs[GPU]);
            get_msg();
            llc.rcv_req();
            // get_msg();
        }
        if (step == 2)
        {
            get_msg();
            tus[ACC].rcv_fwd();
            get_msg();
        }
    }
}

void init_d()
{
    // ------------- Fig 1(d) --------------
    // Handling word granularity ReqWT with line granularity owner.
    // addr: 0xDEADBEEF = 0b 1101 1110 1010 1101 10/11 1110 1110 /11/11

    // Data init;
    // LLC init;
    line_t data_3EE = {CPU, 0x00, 0x00, 0x00, CPU, 0x00, 0x00, 0x00, CPU, 0x00, 0x00, 0x00, CPU, 0x00, 0x00, 0x00};
    LineCopy(llc.cache[0x3EE], data_3EE);
    llc.line_state_buf[0x3EE] = LLC_V;
    llc.word_state_buf[0x3EE].set(); // set all to O state;
    llc.tag_buf[0x3EE] = 0x37AB6;    // 0b 1101 1110 1010 1101 10 -> 11 0111 1010 1011 0110

    // addr: 0xDEADBEEF = 0b 1101 1110 1010 1101 1011 11/10 1110 /11/11
    // CPU init;
    line_t data_CPU_2E = {0x00, 0xAD, 0xBE, 0xEF, 0x01, 0xAD, 0xBE, 0xEF, 0x10, 0xAD, 0xBE, 0xEF, 0x11, 0xAD, 0xBE, 0xEF};
    LineCopy(devs[CPU].cache[0x2E], data_CPU_2E);
    devs[CPU].state_buf[0x2E] = DEV_E;  // not O;
    devs[CPU].tag_buf[0x2E] = 0x37AB6F; // 0b 1101 1110 1010 1101 1011 11 -> 11 0111 1010 1011 0110 1111

    // GPU init (not necessary, but to test if ReqWT works);
    line_t data_GPU_2E = {0x00, 0x37, 0xAB, 0x6F, 0x01, 0x37, 0xAB, 0x6F, 0x10, 0x37, 0xAB, 0x6F, 0x11, 0x37, 0xAB, 0x6F};
    LineCopy(devs[GPU].cache[0x2E], data_GPU_2E);
    devs[GPU].state_buf[0x2E] = DEV_V;
    // should be I??? a store miss??? but assume Valid;
    // Yes it's Owned in CPU; but just assume it is the data to be stored;
    devs[GPU].tag_buf[0x2E] = 0x37AB6F; // 0b 1101 1110 1010 1101 1011 11 -> 11 0111 1010 1011 0110 1111

    // Req init;
    // ReqWTdata;
    Req_001.id = 5;
    Req_001.mask = 0x5; // 0b0101
    Req_001.addr = 0xDEADBEEF;
    Req_001.msg = WRITE;
    Req_001.time_sp = TIME_L1;
    Req_001.time_hm = TIME_L1;
    // Req_001.msg_display();
    // ------------------------------------
}

void do_d()
{
    init_d();
    int step = 0;
    for (step = 1; step < 3; step++)
    {
        cout << "Steps: " << step << endl;
        if (step == 1)
        {
            devs[GPU].req_buf.push_back(Req_001);
            tus[GPU].mapping_wrapper(devs[GPU]);
        }
        get_msg(); // 上升沿get_msg();
        llc.rcv_req();
        get_msg(); // 下降沿get_msg();
        if (step == 1)
            tus[CPU].rcv_fwd();
    }
}

int main()
{
    reset();
    init();

    // Just uncomment the following lines to see the corresponding case.
    // do_a();
    do_b();
    // do_c();
    // do_d();

    return 0;
}