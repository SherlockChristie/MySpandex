#include "classes.hpp"

// 在上升沿时执行所有msg的入队操作，在下降沿时执行所有msg的出队操作;
// 如果在下一个上升沿时到来时，req队列仍然不为空，证明此时数据处于阻塞状态，后来的req加入队列等待;

// 1. Generate the message: rsp/req/fwd;
// 2. Write msg to its own fifo;
// 3. Send msg to the client's fifo;
// 4. the receiver generates message in its own fifo.

// It's a fuxking bus-based system, I should send msg to the BUS first instead of point-to-point.
