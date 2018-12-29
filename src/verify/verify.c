#include <stdlib.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/md5.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "log.h"
#include "verify.h"

#ifndef WIN32 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#else

#include <winsock2.h>
#include <Iphlpapi.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
// #pragma comment(lib, "D:\\Program Files\\OpenSSL-Win64\\lib\\libcrypto.lib")
// #pragma comment(lib, "D:\\Program Files\\OpenSSL-Win64\\lib\\libssl.lib")
#endif 

#ifdef _cplusplus
extern "C"{
#endif
static int get_mac(char *mac); //获取本机MAC地址 
size_t Base64_encode(const char* input, int length, bool with_new_line, char* ret_buf);
size_t Base64_decode(char* input, int length, bool with_new_line, char* ret_buf);
static void byte2hex(unsigned char bData,unsigned char hex[]);

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
		printf( "Failed to create key BIO\n");
		return 0;
	}
 
	if(flag)
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
 
	if(rsa == NULL)
		printf( "Failed to create RSA\n");
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

#if 0
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
#endif

// #ifndef WIN32 
/*
int verify_get_host_ID(char* id, int size)
{
    char mac[1024]={0};
    exec_cmd("ifconfig | grep HWaddr | awk '{print $5}'",id,1023);
    if(strlen(mac)>2)
    {
        return exec_cmd("ifconfig | grep HWaddr | awk '{print $5}'|md5sum|awk '{print $1}'|base64",id,size);
    }
    else
    {
        return exec_cmd("ifconfig | grep ether | awk '{print $2}'|md5sum|awk '{print $1}'|base64",id,size);
    }
    
} */
// #else

static char *md5(const char* src, int size, char* dst)
{
    unsigned char md5[20];
    MD5((unsigned char *)src, size, md5);
    int i;
    for(i=0;i<15;i++)
    {
        byte2hex(md5[i], (unsigned char*)&dst[2*i]);
    }

    return dst;
}

int verify_get_host_ID(char* id, int size)
{
    char mac[1024]={0};
    char mac_md5[20]={0};

    get_mac(mac);
    // LOG_DEBUG("mac:%s",mac)
    md5(mac, strlen(mac), mac_md5);
    // LOG_DEBUG("mac_md5:%s",mac_md5);
    return Base64_encode( mac_md5, strlen(mac_md5), 0, id );
}
// #endif 

#ifdef WIN32
int verify_auth(const char* file_name)
{
    int len = 0;
	FILE* fd = NULL;

	fopen_s(&fd, file_name, "r");
	if(fd==NULL) {
		printf("open %s error.\n", file_name);
		return -1;
	}
    char host_key[1024] = {0};
    char de_str[1024] = {0};
    unsigned char* key = NULL;
    int ret = -1;
    char public_key[1024]={0};

    if (fread(&len, sizeof(int), 1, fd)<0) {
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

    if (fread( key, len, 1, fd) < 0) {

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
    LOG_ERROR("host_key:%s",host_key);
    LOG_ERROR("de_str:%s",de_str);
    if (memcmp(host_key,de_str,strlen(host_key)) == 0) {
        ret = 0;
    }

END:
    if(key!=NULL) free(key);
    fclose(fd);
    return ret;

}
#else

int verify_auth(const char* file_name)
{
    int len = 0;
	int fd = 0;
    char host_key[1024] = {0};
    char de_str[1024] = {0};
    unsigned char* key = NULL;
    int ret = -1;
    char public_key[1024]={0};

    fd = open(file_name,O_RDONLY);
    if( fd < 0 )
    {
        LOG_ERROR("open error:%d,%s",errno,strerror(errno));
        ret = -1;
        goto END;
    }
    if (read(fd, &len, sizeof(int))<0) {
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
    // LOG_ERROR("host_key:%s",host_key);
    // LOG_ERROR("de_str:%s",de_str);
    if (memcmp(host_key,de_str,strlen(host_key)) == 0) {
        ret = 0;
    }

END:
    if(key!=NULL) free(key);
    close(fd);
    return ret;

}
#endif

static void byte2hex(unsigned char bData,unsigned char hex[])
{
	int high=bData/16,low =bData %16;
	hex[0] = (high <10)?('0'+high):('a'+high-10);
    hex[1] = (low <10)?('0'+low):('a'+low-10);
}

#ifdef WIN32
static int get_mac(char *mac) //获取本机MAC地址 
{
    ULONG ulSize=0;
    PIP_ADAPTER_INFO pInfo=NULL;
    int temp=0;
    temp = GetAdaptersInfo(pInfo,&ulSize);//第一次调用，获取缓冲区大小
    pInfo=(PIP_ADAPTER_INFO)malloc(ulSize);
    temp = GetAdaptersInfo(pInfo,&ulSize);

    int iCount=0;
    while(pInfo)//遍历每一张网卡
    {
        //  pInfo->Address 是MAC地址
        for(int i=0;i<(int)pInfo->AddressLength;i++)
        {
            byte2hex((unsigned char)pInfo->Address[i],&mac[iCount]);
            iCount+=2;
            if(i<(int)pInfo->AddressLength-1)
            {
                mac[iCount++] = ':';
            }else
            {
                mac[iCount++] = ';';
            }
        }
        pInfo = pInfo->Next;
    }

    if(iCount >0)
    {
        mac[--iCount]='\0';
        return iCount;
    }
    else return -1;
}

#else

static int get_mac(char* mac)
{
    int fd = 0;
    struct ifreq buf[16];
    struct ifconf ifc;
    // struct ifreq ifrcopy;
    char mac_addr[20] = {0};

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        close(fd);
        return -1;
    }

    if ( ioctl(fd, SIOCGIFCONF, (char *)&ifc) < 0 )
    {
        LOG_ERROR("ioctl error:%d,%s", errno, strerror(errno));
        return -1;
    }
    int interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
    // LOG_DEBUG("interfaceNum:%d",interfaceNum);
    for (; interfaceNum > 0; interfaceNum--)
    {
        // LOG_DEBUG("ndevice name: %s", buf[interfaceNum].ifr_name);
        if( ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum]) ) < 0 )
        {
            // LOG_ERROR("ioctl error:%d,%s", errno, strerror(errno));
            // return -1;
            continue;
        }
        memset(mac_addr, 0, sizeof(mac_addr));
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[0],(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[1], (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[2],(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[3],(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[4], (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[5]);
        // LOG_DEBUG("mac_addr:%s",mac_addr);
        strcat(mac,mac_addr);
        strcat(mac,";");
    }
    close(fd);
    fd = strlen(mac);
    if(fd>0)
    {
        mac[fd-1]=0;
    }

    return fd;
}

#endif



#if 0
int main(int argc, char* argv[])
{
    char address[1024];
    if(get_mac(address)>0)
    {
        printf("MAC-%s\n",address);
    }else
    {
        printf("invoke getMAC error!\n");
    }
    return 0;
}
#endif

size_t Base64_encode(const char* input, int length, bool with_new_line, char* ret_buf)
{
	BIO * bmem = NULL;
	BIO * b64 = NULL;
	BUF_MEM * bptr = NULL;
    size_t ret_len = 0;
 
	b64 = BIO_new(BIO_f_base64());
	if(!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
    
	memcpy(ret_buf, bptr->data, bptr->length);
    ret_len = bptr->length;
	BIO_free_all(b64);
 
	return ret_len;
}

size_t Base64_decode(char* input, int length, bool with_new_line, char* ret_buf)
{
	BIO * b64 = NULL;
	BIO * bmem = NULL;
    int ret = 0;
 
	b64 = BIO_new(BIO_f_base64());
	if(!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);
	ret = BIO_read(bmem, ret_buf, length);
 
	BIO_free_all(bmem);
 
	return ret;
}

#ifdef _cplusplus
}
#endif