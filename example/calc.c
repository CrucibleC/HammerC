#include "calc.h"

int calc_add(int a, int b)
{
    return a + b;
}

int calc_clamp(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}
