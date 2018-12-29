// cstd_enc_key.cpp : 定义控制台应用程序的入口点。
//
#include<stdio.h>
#include "stdafx.h"
#include <errno.h>
#include <string.h>
#include "verify.h"

static unsigned char private_key[] = "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEowIBAAKCAQEAv/aLRjWYuTN0VXE9Hkwggb1WQGF5xVCa3GCWf+M4+D4XC/IF\n"\
"mUwDG3b2dx4qjOmO9xwFD+BJv6SBhyuz0TyYLw/JlQo7ZoT6ItRs0JlD0fh6FXIN\n"\
"HslvpKiQN0gLOoSP3dDrnGDFwjrBUguJvX5lzpsDDKmsE1+CUm+j7kX7hTfUIkCt\n"\
"mLRKKc6pO/U2triasnIiMr3mhPu5/ojnR044NIEipTKvLjq4AewiUF8ywxLUjSut\n"\
"nURev8BtxvcPlaT6Ou9OcGo9YbuyENm4FibTumJitLQ9PDhEh8W0GNHXELf9H3Jz\n"\
"QN5SQAN8Dca87bLO4WLzaZWQXsO5pd2VEETWEQIDAQABAoIBADo0J9B26cKlvk01\n"\
"04E/NdAAt4k3xPM12G6ffqJyI9DpCpXdoSL08DBApsRBSkeFfYqhXUuOBu5rk7aH\n"\
"YrM7ey0xzT46Zv5TrYAoBoJNJkog2+tqtF20V/yF+4w3dOgeT3zDHt1AXlXg+MLG\n"\
"qtHg5D97srYBMCKDoAEcxoi1gWjHmMiiLwL3oyEddYQJYa4ed+0RIXyJ+nlsBgrm\n"\
"S34YuX336QCn0AjaqmZkfiubn7GJwxtq1/YVsQRkCkWXJFWdwu/RtkzJ3rB458ey\n"\
"gp6ZV9q8oQwHBEbP0KhXr8ohiOaDcqR1yJ2P4tYet1I3dfoqlwB3J9C/04xacaPt\n"\
"SF64+BECgYEA8Vx7lWYmVXrM1LvL2c/gHYzDqZv1BMLfiJ2GvfSn5wPzqH+G/LQA\n"\
"KrymvZHds845/lT2EThq4a//j4zvwl+agOAvccbkVooT2y6z+40M3gB2+jkKiKsl\n"\
"TNz+0YbPcroOLtvpXjVNOpKVBn5HTOpiI7pjzcN8JR2KgGotIMxhki0CgYEAy5sT\n"\
"Y8FAkNfC/bhb5OBZxSgkNyWqRX53q3XMIzYSed2UkC7Oo99QN8peInwvsn/murY7\n"\
"hPXkVbPe01usgKS+fsvrShJm80vDZ+0ooY6Tw4HE0CuvD4/tIvpgij8Gb9RijBw8\n"\
"jj6xfQQM4IEFH3j+sPY+uLXpVd3Unpr/2U6nVfUCgYAg1+rhO5M/0oQI9r0YmJbw\n"\
"rnT5oYVH7pSobHcnZiFq6DjB1wixXnUfW6h851p52zg9CaaqVTkGGQL3r7/4fTXz\n"\
"D46lNuFffU6SZtqLAnibxQhrqFNsoY1j1ja+oE+eVv8TFU2+ShaGP+gTiB9/se1E\n"\
"21o1Z8J+8nQkYU5exjEIyQKBgHr7ptCgkIFs8mkTDN7I7TB7uLRe2eS0KVC2DKJQ\n"\
"LQTTZMNaopK1dtzcLdQisyTcwN+S54y+gjeKOFPukdn6qN4qCmtphin7X4LM67f6\n"\
"SSLUpa8gU9PQmeqA1gNrYlX2Mbhz7df14nujEKwjApcqHJ2DgAtORi0Jmz5aXaRD\n"\
"MCxpAoGBAMYF8d7uq34j5oKBrFnhal6c4Chl/bQi+bNlBDHtCHaYObsS54v5nYoI\n"\
"lnDbTtyyeCW0JgNj7rTn9Uj+NnLJOOSG844RFf1vovhwktT2jZzynWBJuaF25Ghv\n"\
"vtBZ6G8GXUfgzMQ0SBjsz7X7xoPib7YTAHUzdbElvCK9uvExMPo7\n"\
"-----END RSA PRIVATE KEY-----";

int main(int argc, char** argv)
{
	int ret = 0;
	unsigned char ret_buf[1024] = { 0 };
	if (argc != 3)
	{
		printf("usage:%s host_key output_file\n", argv[0]);
		return 0;
	}
	ret = private_encrypt((unsigned char*)argv[1], strlen(argv[1]), private_key, ret_buf);
	if (ret<0)
	{
		printf("encrypt failed.\n");
		return -1;
	}
	FILE* fd = fopen(argv[2], "w");
	if (fd==NULL) {
		printf("creat %s failed.%s\n", argv[2], strerror(errno));
		return -1;
	}
	if (fwrite( &ret, sizeof(ret), 1, fd ) < 0) {
		printf("fwrite %s failed.%s\n", argv[2], strerror(errno));
		goto end;
	}
	if (fwrite( ret_buf, ret, 1, fd) < 0) {
		printf("fwrite %s failed.%s\n", argv[2], strerror(errno));
		goto end;
	}

end:
	fclose(fd);
	return 0;
}