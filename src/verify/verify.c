#include <stdlib.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

static const unsigned char public_key2[]="MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAv/aLRjWYuTN0VXE9Hkwg\n";
static const unsigned char public_key7[]="umJitLQ9PDhEh8W0GNHXELf9H3JzQN5SQAN8Dca87bLO4WLzaZWQXsO5pd2VEETW\n";
static const unsigned char public_key1[]= "-----BEGIN PUBLIC KEY-----\n";
static const unsigned char public_key9[]="-----END PUBLIC KEY-----";
static const unsigned char public_key8[]="EQIDAQAB\n";
static const unsigned char public_key4[]="Lw/JlQo7ZoT6ItRs0JlD0fh6FXINHslvpKiQN0gLOoSP3dDrnGDFwjrBUguJvX5l\n";
static const unsigned char public_key6[]="NIEipTKvLjq4AewiUF8ywxLUjSutnURev8BtxvcPlaT6Ou9OcGo9YbuyENm4FibT\n";
static const unsigned char public_key3[]="gb1WQGF5xVCa3GCWf+M4+D4XC/IFmUwDG3b2dx4qjOmO9xwFD+BJv6SBhyuz0TyY\n";
static const unsigned char public_key5[]="zpsDDKmsE1+CUm+j7kX7hTfUIkCtmLRKKc6pO/U2triasnIiMr3mhPu5/ojnR044\n";


/*
static const unsigned char private_key[] = "-----BEGIN RSA PRIVATE KEY-----\n"\
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
*/

const static int padding = RSA_PKCS1_PADDING;

static RSA* create_RSA(const unsigned char* key, int flag)
{
	RSA *rsa= NULL;
	BIO *keybio ;
	keybio = BIO_new_mem_buf(key, -1);
 
	if (keybio==NULL) {
		printf( "Failed to create key BIO");
		return 0;
	}
 
	if(flag)
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
 
	if(rsa == NULL)
		printf( "Failed to create RSA");
    BIO_free(keybio);
	return rsa;
}
#if 0
static int public_encrypt(unsigned char* data, int data_len, unsigned char* key, unsigned char* encrypted)
{
	RSA * rsa = create_RSA(key, 1);
	int result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
    RSA_free(rsa);
	return result;
}
 
static int private_decrypt(unsigned char* enc_data, int data_len, unsigned char* key, unsigned char* decrypted)
{
	RSA * rsa = create_RSA(key, 0);
	int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    RSA_free(rsa);
	return result;
}
 #endif
int private_encrypt(unsigned char* data, int data_len, unsigned char* key, unsigned char* encrypted)
{
	RSA * rsa = create_RSA(key, 0);
	int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
    RSA_free(rsa);
	return result;
}
 
static int public_decrypt(unsigned char* enc_data, int data_len, unsigned char* key, unsigned char* decrypted)
{
	RSA * rsa = create_RSA(key, 1);
	int  result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
    RSA_free(rsa);
	return result;
}

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

int verify_auth(const char* file_name)
{
    int len = 0;
    int fd = open(file_name,O_RDONLY);
    char host_key[1024] = {0};
    char de_str[1024] = {0};
    unsigned char* key = NULL;
    int ret = 0;
    char public_key[1024]={0};

    if( fd < 0 )
    {
        LOG_ERROR("open error:%d,%s",errno,strerror(errno));
        ret = -1;
        goto END;
    }
    if (read(fd,&len,sizeof(int))<0) {
        LOG_ERROR("read error:%d,%s",errno,strerror(errno));
        ret = -1;
        goto END;
    }
    if (len <= 0) {
        LOG_ERROR("len error.");
        ret = -1;
        goto END;
    }
    key = (unsigned char*)calloc(1, len);
    if (key == NULL) {
        LOG_ERROR("calloc error.");
        ret = -1;
        goto END;
    }

    if (read(fd, key, len) < 0) {

        LOG_ERROR("read error:%d,%s",errno,strerror(errno));
        ret = -1;
        goto END;
    }
    sprintf(public_key,"%s%s%s%s%s%s%s%s%s",public_key1,public_key2,public_key3,public_key4,public_key5,public_key6,public_key7,public_key8,public_key9);
    if (public_decrypt(key,len,(unsigned char*)public_key,(unsigned char*)de_str) < 0) {
        
        LOG_ERROR("public_decrypt error.");
        ret = -1;
        goto END;
    }
    verify_get_host_ID(host_key,sizeof(host_key));
    if (memcmp(host_key,de_str,strlen(host_key)) == 0) {
        ret = 0;
    }

END:
    if(key!=NULL) free(key);
    close(fd);
    return ret;

}
