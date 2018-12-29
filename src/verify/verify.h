#ifndef __VERIFY_H__
#define __VERIFY_H__

int verify_get_host_ID(char* id, int size);

int private_encrypt(unsigned char* data, int data_len, unsigned char* key, unsigned char* encrypted);

int verify_auth(const char* file_name);

#endif 
