// cstd_genkey.cpp : �������̨Ӧ�ó������ڵ㡣
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

