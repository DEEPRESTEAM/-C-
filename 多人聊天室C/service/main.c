#include"work.h"

int main(int argc, char const *argv[])
{
    int fd = serviceInit();
    struMain(fd);
    return 0;
}
