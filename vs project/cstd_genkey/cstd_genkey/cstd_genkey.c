// cstd_genkey.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include "stdafx.h"
#include "verify.h"

int main()
{
	char key[512] = { 0 };

	verify_get_host_ID(key, sizeof(key));
	printf("%s", key);
    return 0;
}

