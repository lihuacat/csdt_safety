
#include <stdio.h>

static int exec_cmd(const char* cmd, char *ret, int size)
{
    int iret = 0;
    FILE* pf = popen(cmd, "r");
    if (pf == NULL) {
        return -1;
    }

    iret = fread(ret, size, 1, pf);

    fclose(pf);

    return iret;
}

int verify_get_host_ID(char* id, int size)
{
    return exec_cmd("ifconfig | grep HWaddr | awk '{print $5}'|md5sum|awk '{print $1}'|base64",id,size);
}

int main(int argc, char** argv)
{
    char key[512]={0};

    verify_get_host_ID(key,sizeof(key));
    printf("%s",key);
    return 0;
}