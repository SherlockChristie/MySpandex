#include <cstdio>
class Stu
{
    // something
};

void fun(Stu &arr)
{
    printf("OK\n");
}

int main()
{
    Stu arr[4];
    fun(arr[2]);
    return 0;
}