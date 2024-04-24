#include "bit_utils.hpp"
#include "blocks.hpp"
#include "classes.hpp"
#include "consts.hpp"
#include <vector>
using namespace std;

std::vector<MSG> bus;
DEV cpu, gpu, acc;
TU tcpu, tgpu, tacc;
LLC llc;

int main()
{
    // init;
    // reset;

    return 0;
}

void get_msg()
{
    MSG tmp;
    tmp = bus.front();
    // for (int i = 0; i < MAX_DEVS; i++)
    {
        if (tmp.msg < RSP_S) // req or fwd
        {
            if (tmp.dest.test(0))
                llc.req_buf.push_back(tmp);
            if (tmp.dest.test(1))
                tcpu.req_buf.push_back(tmp);
            if (tmp.dest.test(2))
                tgpu.req_buf.push_back(tmp);
            if (tmp.dest.test(3))
                tacc.req_buf.push_back(tmp);
        }
        else // rsp;
        {
            // if (tmp.dest.test(0))
            //     llc.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(1))
            //     tcpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(2))
            //     tgpu.rsp_buf.push_back(tmp);
            // if (tmp.dest.test(3))
            //     tacc.rsp_buf.push_back(tmp);

            // 无需将rsp_in入队，直接对对碰消掉req和rsp_in;
            if (tmp.dest.test(0))
                get_rsp(tmp, llc.req_buf);
            if (tmp.dest.test(1))
                get_rsp(tmp, tcpu.req_buf);
            if (tmp.dest.test(2))
                get_rsp(tmp, tgpu.req_buf);
            if (tmp.dest.test(3))
                get_rsp(tmp, tacc.req_buf);
        }
    }
    bus.erase(bus.begin());
}

void put_req(std::vector<MSG> &req)
// Put all the req_buf to the bus.
{
    int req_len = req.size();
    for (int i = 0; i < req_len; i++)
    // while (!req.empty())
    {
        MSG tmp;
        tmp = req.front();
        bus.push_back(tmp);
        // all req stays at the vector until its rsp in;
    }
}
void put_rsp(std::vector<MSG> &rsp)
// Put all the rsp_buf to the bus.
{
    int rsp_len = rsp.size();
    for (int i = 0; i < rsp_len; i++)
    // while (!rsp.empty())
    {
        MSG tmp;
        tmp = rsp.front();
        bus.push_back(tmp);
        if (tmp.msg < FWD_REQ_S) // rsp, not fwd;
        {
            rsp.erase(rsp.begin());
        }
    }
}

void get_rsp(MSG rsp, std::vector<MSG> &req)
// get a rsp, find match in req_buf;
{
    int req_len = req.size();
    for (int i = 0; i < req_len; i++)
    {
        if (rsp.id == req[i].id)
        {
            if (rsp.gran = GRAN_LINE)
            { // 同时释放req和rsp;
                // 但是rsp已经在总线释放了？
                req.erase(req.begin() + i);
            }
            else
            {
                req[i].mask.set(rsp.offset.to_ulong());
                if (req[i].mask.all()) // 收集到了所有rsp;
                {
                    req.erase(req.begin() + i);
                }
            }
        }
    }
}

// void get_req(std::vector<MSG> &req)
// // get a req, insert it in req_buf;
// {
//     req.push_back(bus.front()); // req_buf入队;
//     bus.erase(bus.begin());     // bus出队;
// }