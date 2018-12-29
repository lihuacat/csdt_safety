
#include <stdio.h>
#include <string.h>
#include "verify.h"

int main(int argc, char** argv)
{
    char key[512]={0};

    verify_get_host_ID(key,sizeof(key));
    printf("%s\n",key);
    return 0;
}