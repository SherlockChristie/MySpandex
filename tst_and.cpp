#include <cstdio>

int main()
{
    int a = 3;
    int b = 4;
    int *pointerToA = &a;
    int *pointerToB = &b;
    int *p = pointerToA;
    p = pointerToB;
    printf("%d %d %d\n", a, b, *p); // Prints 3 4 4
    int &referenceToA = a;
    int &referenceToB = b;
    int &r = referenceToA;
    r = referenceToB;
    printf("%d %d %d\n", a, b, r); // Prints 4 4 4
    return 0;
}