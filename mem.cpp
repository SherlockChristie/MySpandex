#include <iostream>
#include <vector>
#include <cstdint> // For uint8_t, uint32_t etc.

class Memory {
public:
    Memory(size_t size) : memory(size, 0) {}

    // 读取一个字节
    uint8_t readByte(uint32_t address) {
        if (address < memory.size()) {
            return memory[address];
        } else {
            // 在实际的RISC-V模拟器中，你可能想要处理这个条件，比如抛出一个异常
            std::cerr << "Read access violation at address " << address << std::endl;
            return 0;
        }
    }

    // 写入一个字节
    void writeByte(uint32_t address, uint8_t data) {
        if (address < memory.size()) {
            memory[address] = data;
        } else {
            // 在实际的RISC-V模拟器中，你可能想要处理这个条件
            std::cerr << "Write access violation at address " << address << std::endl;
        }
    }

    // 模拟读取一个32位整数
    uint32_t readWord(uint32_t address) {
        uint32_t word = 0;
        for (int i = 0; i < 4; i++) {
            word |= readByte(address + i) << (i * 8);
        }
        return word;
    }

    // 模拟写入一个32位整数
    void writeWord(uint32_t address, uint32_t data) {
        for (int i = 0; i < 4; i++) {
            writeByte(address + i, (data >> (i * 8)) & 0xFF);
        }
    }

private:
    std::vector<uint8_t> memory;
};

int main() {
    Memory mem(1024); // 创建一个1KB的内存模型

    // 写入并读取一个字节
    mem.writeByte(100, 0xAB);
    std::cout << "Read byte: 0x" << std::hex << static_cast<int>(mem.readByte(100)) << std::endl;

    // 写入并读取一个字（32位整数）
    mem.writeWord(200, 0xDEADBEEF);
    std::cout << "Read word: 0x" << std::hex << mem.readWord(200) << std::endl;

    return 0;
}
