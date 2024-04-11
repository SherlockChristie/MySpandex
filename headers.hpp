// #include <cmath>

#ifndef __SPANDEX_CONSTS_SVH__
#define __SPANDEX_CONSTS_SVH__

#include "bit_utils.hpp"

#define ADDR_SIZE 32     // 32-bit address;
#define BITS_PER_BYTE 8
#define BYTES_PER_WORD 4 // uint8_t for byte, uint32_t for word; // no, all use bitset instead;
#define WORDS_PER_LINE 4
#define BYTES_OFF lg2(BYTES_PER_WORD) // 2
#define WORDS_OFF lg2(WORDS_PER_LINE) // 2

#define MAX_RETRY 1

// #define WORD_MASK ((1 << BYTES_PER_WORD) - 1)

#define DEV_ROW 64                             // 2^6
#define DEV_COL BYTES_PER_WORD*WORDS_PER_LINE // byte addressing;

#define LLC_ROW 1024 // 2^10
#define LLC_COL DEV_COL

#define MEM_ROW 16384 // 2^14
#define MEM_COL DEV_COL

#define DEV_INDEX_BITS lg2(DEV_ROW) // 6
#define LLC_INDEX_BITS lg2(LLC_ROW) // 10

// #define DEV_TAG_BITS lg2(LLC_ROW/DEV_ROW) // 4
// #define LLC_TAG_BITS lg2(MEM_ROW/LLC_ROW) // 4
#define DEV_TAG_BITS ADDR_SIZE - DEV_INDEX_BITS - WORDS_OFF - BYTES_OFF // 22
#define LLC_TAG_BITS ADDR_SIZE - LLC_INDEX_BITS - WORDS_OFF - BYTES_OFF // 18
// 仅后4位有值，高位全为0; 但是保持地址位宽一致;

#define STATE_LINE 2               // 2 bits for a line (Invalid, Valid or Shared);
#define STATE_WORDS WORDS_PER_LINE // 1 bit for a word;
#define STATE_BITS STATE_LINE+STATE_WORDS // no need to separate them;
// Described in Section III-B:
// To limit tag and state overhead, allocation occurs at line granularity.
// For each line, two bits indicate whether the line is Invalid, Valid or Shared.
// For each word within the cache line, a single bit tracks whether the word is Owned in a remote cache.
// For each Owned word, the data field itself stores the ID of the remote owner.
#define MAX_DEVS 2 // 2 bits for both sharers and owners ID;
// 为方便计假设只有三种设备且每种设备都只有一个 Cache，用下文定义的 Device type 区分;
// 实际上 MAX_DEVS = lg2(3); 不过上文定义的lg2函数无法处理3;

// Granularity type
#define GRAN_WORD 0
#define GRAN_LINE 1

/// Device type
// same with TU's type
#define CPU 0
#define GPU 1
#define ACC 2

/// Request (DEV to TU)
#define READ 0   // read
#define WRITE 1  // write
#define RMW 2    // read, write and modify
#define O_REPL 3 // owned replacement

/// Device states
#define DEV_I 0
#define DEV_V 1
#define DEV_S 2
#define DEV_O 3

/// Device Transient states
#define DEV_IV 1
#define DEV_II 2
#define DEV_OI 3
#define DEV_AMO 4
#define DEV_IV_DCS 5
#define DEV_XO 6
#define DEV_XOV 7
#define DEV_IS 8

// /// TU type
// #define TU_CPU 0
// #define TU_GPU 1
// #define TU_ACC 2

/// LLC states
#define LLC_I 0
#define LLC_V 1
#define LLC_S 2
// O means the data is Owned within a device cache, not LLC.
// ???
#define LLC_O 3

/// LLC unstable states
#define LLC_IV 1
#define LLC_IS 2
#define LLC_IO 3
#define LLC_SO 4
#define LLC_SV 5
#define LLC_OS 6
#define LLC_OV 7
#define LLC_SWB 8
#define LLC_OWB 9
#define LLC_SI 10
#define LLC_WB 11

/// Request (TU to LLC)
#define REQ_S 0     /// same as gets
#define REQ_Odata 1 /// same as getm
#define REQ_WT 2
#define REQ_WB 3 /// same as putm
#define REQ_O 4
#define REQ_V 5
#define REQ_WTdata 6
#define REQ_WTfwd 7

/// Forward (LLC to DEV/TU)
#define FWD_REQ_S 0     /// same as fwd_gets
#define FWD_REQ_Odata 1 /// same as fwd_getm
#define FWD_INV 2       /// same as fwd_inv
#define FWD_WB_ACK 3    /// same as fwd_putack
#define FWD_RVK_O 4     /// same as getm_llc
#define FWD_REQ_V 7     /// non existent in ESP
#define FWD_REQ_O 6
#define FWD_WTfwd 5

/// Response (DEV/TU to DEV/TU, DEV/TU to LLC, LLC to DEV/TU)
#define RSP_S 0
#define RSP_Odata 1   /// same as fwd_req_odata
#define RSP_INV_ACK 2 /// same as fwd_inv_spdx
#define RSP_NACK 3
#define RSP_RVK_O 4 /// same as fwd_rvk_o
#define RSP_V 5
#define RSP_O 6 /// same as fwd_req_o
#define RSP_WT 7
#define RSP_WTdata 8
#define RSP_WB_ACK 9

#endif /// __SPANDEX_CONSTS_SVH__