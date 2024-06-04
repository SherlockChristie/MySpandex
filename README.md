### 函数

主要的函数功能如本节所述。

#### 互连网络

- 输入指向 buf 的指针，将 buf 内容发送至网络的函数 put_msg
- 将网络消息发送至 dest buf 的函数 get_msg

#### 设备

- 接受 32 位地址，并分解为设备地址格式的函数 breakdown
- 根据地址取出数据，返回是否成功的函数 fetch_line
- 根据地址写回数据，返回是否成功的函数 back_line

#### LLC

- breakdown, fetch_line 与 back_line 的功能与“设备”中的一样，仅分解出的数据格式不同
- 处理请求函数 rcv_req
  - 处理单字请求函数 rcv_req_single
- 处理响应函数 rcv_rsp

#### TU

- 映射函数 mapping_wrapper
  - 数据状态映射 data_mapping
  - 核心事件映射 req_mapping
- 处理转发请求函数 rcv_fwd
  - 处理单字转发请求函数 rcv_fwd_single
- 处理响应函数 rcv_rsp

#### 其他函数

- 设备、TU 和 LLC 共用的接收响应内层函数 rcv_rsp_inner
- 接受指向 buf 的指针，检查 buf 内是否有指向同一地址的请求，若有，返回序号的函数 find_conflict
- 接受指向 buf 的指针，合并 buf 内子行粒度的消息的函数 MsgCoalesce
- 接受目标指针、数据行及偏移量，从行中提取字及状态 WordIns
- 接受目标指针、数据字及偏移量，为行中插入字及状态 WordExt
- 接受数据行，提取拥有者 FindOwner
- 接受数据行，提取共享者 InvSharers
- 数据拷贝与移动、bitset 分割与连接、显示与打印函数若干

各个处理函数需要实现的转换如`LLC控制器`和`缓存控制器`所述。