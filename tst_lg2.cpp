#include <iostream>

#include <cstdio>
using namespace std;

int lg2(int x) {
    int result = 0; // 初始化结果
    x--; // 减1后，完全二的幂的值也需要向上取整

    // 循环移位，直到x变为0
    while (x > 0) {
        x >>= 1;
        result++;
    }
    return result;
}
int main()
{
    int a=5;
    cout<<lg2(a);
    return 0;
}