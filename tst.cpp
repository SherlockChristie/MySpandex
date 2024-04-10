#include <iostream>

#include <cstdio>
using namespace std;

int lg2(int x) {
    int result = 0; // 初始化结果为0
    while (x > 1) {
        x >>= 1;  // 通过右移一位来除以2
        result++; // 每移一次，实际上就是log2(x)的值加1
    }
    return result;
}
int main()
{
    int a=16384;
    cout<<lg2(a);
    return 0;
}