#include <stdio.h>
#include <time.h>

int main(void)
{
    struct tm t;//1858-11-17
    t.tm_sec=0;
    t.tm_min=0;
    t.tm_hour=0;
    t.tm_mday=17;
    t.tm_mon=10;
    t.tm_year=-42;
    printf("%s",asctime(&t));
    return 0;
}
