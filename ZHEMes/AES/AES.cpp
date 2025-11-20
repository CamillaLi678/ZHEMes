#include "stdafx.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    uint32_t eK[44], dK[44];    // encKey, decKey
    int Nr; // 10 rounds
}AesKey;
#define BLOCKSIZE 16  //AES-128分组长度为16字节
// uint8_t y[4] -> uint32_t x
#define LOAD32H(x, y) \
  do { (x) = ((uint32_t)((y)[0] & 0xff)<<24) | ((uint32_t)((y)[1] & 0xff)<<16) | \
             ((uint32_t)((y)[2] & 0xff)<<8)  | ((uint32_t)((y)[3] & 0xff));} while(0)

// uint32_t x -> uint8_t y[4]
#define STORE32H(x, y) \
  do { (y)[0] = (uint8_t)(((x)>>24) & 0xff); (y)[1] = (uint8_t)(((x)>>16) & 0xff);   \
       (y)[2] = (uint8_t)(((x)>>8) & 0xff); (y)[3] = (uint8_t)((x) & 0xff); } while(0)

// 从uint32_t x中提取从低位开始的第n个字节
#define BYTE(x, n) (((x) >> (8 * (n))) & 0xff)

/* used for keyExpansion */
// 字节替换然后循环左移1位
#define MIX(x) (((S[BYTE(x, 2)] << 24) & 0xff000000) ^ ((S[BYTE(x, 1)] << 16) & 0xff0000) ^ \
                ((S[BYTE(x, 0)] << 8) & 0xff00) ^ (S[BYTE(x, 3)] & 0xff))

// uint32_t x循环左移n位
#define ROF32(x, n)  (((x) << (n)) | ((x) >> (32-(n))))
// uint32_t x循环右移n位
#define ROR32(x, n)  (((x) >> (n)) | ((x) << (32-(n))))

/* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
// AES-128轮常量
static const uint32_t rcon[10] = {
        0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL, 0x10000000UL,
        0x20000000UL, 0x40000000UL, 0x80000000UL, 0x1B000000UL, 0x36000000UL
};
// S盒
static unsigned char S[256] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

//逆S盒
static unsigned char inv_S[256] = {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
        0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
        0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
        0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
        0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
        0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
        0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
        0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
        0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
        0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
        0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

/* copy in[16] to state[4][4] */
static int loadStateArray(uint8_t (*state)[4], const uint8_t *in) {
    int i=0,j=0;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[j][i] = *in++;
        }
    }
    return 0;
}

/* copy state[4][4] to out[16] */
static int storeStateArray(uint8_t (*state)[4], uint8_t *out) {
    int i=0,j=0;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            *out++ = state[j][i];
        }
    }
    return 0;
}
//秘钥扩展
static int keyExpansion(const uint8_t *key, uint32_t keyLen, AesKey *aesKey) {
    int i=0,j=0;
    if (NULL == key || NULL == aesKey){
        printf("keyExpansion param is NULL\n");
        return -1;
    }

    if (keyLen != 16){
        printf("keyExpansion keyLen = %d, Not support.\n", keyLen);
        return -1;
    }

    uint32_t *w = aesKey->eK;  //加密秘钥
    uint32_t *v = aesKey->dK;  //解密秘钥

    /* keyLen is 16 Bytes, generate uint32_t W[44]. */

    /* W[0-3] */
    for (i = 0; i < 4; ++i) {
        LOAD32H(w[i], key + 4*i);
    }

    /* W[4-43] */
    for (i = 0; i < 10; ++i) {
        w[4] = w[0] ^ MIX(w[3]) ^ rcon[i];
        w[5] = w[1] ^ w[4];
        w[6] = w[2] ^ w[5];
        w[7] = w[3] ^ w[6];
        w += 4;
    }

    w = aesKey->eK+44 - 4;
    //解密秘钥矩阵为加密秘钥矩阵的倒序，方便使用，把ek的11个矩阵倒序排列分配给dk作为解密秘钥
    //即dk[0-3]=ek[41-44], dk[4-7]=ek[37-40]... dk[41-44]=ek[0-3]
    for (j = 0; j < 11; ++j) {

        for (i = 0; i < 4; ++i) {
            v[i] = w[i];
        }
        w -= 4;
        v += 4;
    }

    return 0;
}

// 轮秘钥加
static int addRoundKey(uint8_t (*state)[4], const uint32_t *key) {
    uint8_t k[4][4];
    int i=0,j=0;
    /* i: row, j: col */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            k[i][j] = (uint8_t) BYTE(key[j], 3 - i);  /* 把 uint32 key[4] 先转换为矩阵 uint8 k[4][4] */
            state[i][j] ^= k[i][j];
        }
    }

    return 0;
}

//字节替换
static int subBytes(uint8_t (*state)[4]) {
    int i=0,j=0;
    /* i: row, j: col */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = S[state[i][j]]; //直接使用原始字节作为S盒数据下标
        }
    }

    return 0;
}

//逆字节替换
static int invSubBytes(uint8_t (*state)[4]) {
    int i=0,j=0;
    /* i: row, j: col */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = inv_S[state[i][j]];
        }
    }
    return 0;
}

//行移位
static int shiftRows(uint8_t (*state)[4]) {
    uint32_t block[4] = {0};
    int i=0;
    /* i: row */
    for (i = 0; i < 4; ++i) {
    //便于行循环移位，先把一行4字节拼成uint_32结构，移位后再转成独立的4个字节uint8_t
        LOAD32H(block[i], state[i]);
        block[i] = ROF32(block[i], 8*i);
        STORE32H(block[i], state[i]);
    }

    return 0;
}

//逆行移位
static int invShiftRows(uint8_t (*state)[4]) {
    uint32_t block[4] = {0};
    int i=0;
    /* i: row */
    for (i = 0; i < 4; ++i) {
        LOAD32H(block[i], state[i]);
        block[i] = ROR32(block[i], 8*i);
        STORE32H(block[i], state[i]);
    }

    return 0;
}

/* Galois Field (256) Multiplication of two Bytes */
// 两字节的伽罗华域乘法运算
static uint8_t GMul(uint8_t u, uint8_t v) {
    uint8_t p = 0;
    int i=0;
    for (i = 0; i < 8; ++i) {
        if (u & 0x01) {    //
            p ^= v;
        }

        int flag = (v & 0x80);
        v <<= 1;
        if (flag) {
            v ^= 0x1B; /* x^8 + x^4 + x^3 + x + 1 */
        }

        u >>= 1;
    }

    return p;
}

// 列混合
static int mixColumns(uint8_t (*state)[4]) {
    int i=0,j=0;
    uint8_t tmp[4][4];
    uint8_t M[4][4] = {{0x02, 0x03, 0x01, 0x01},
                       {0x01, 0x02, 0x03, 0x01},
                       {0x01, 0x01, 0x02, 0x03},
                       {0x03, 0x01, 0x01, 0x02}};

    /* copy state[4][4] to tmp[4][4] */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j){
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {  //伽罗华域加法和乘法
            state[i][j] = GMul(M[i][0], tmp[0][j]) ^ GMul(M[i][1], tmp[1][j])
                        ^ GMul(M[i][2], tmp[2][j]) ^ GMul(M[i][3], tmp[3][j]);
        }
    }

    return 0;
}

// 逆列混合
static int invMixColumns(uint8_t (*state)[4]) {
    int i=0,j=0;
    uint8_t tmp[4][4];
    uint8_t M[4][4] = {{0x0E, 0x0B, 0x0D, 0x09},
                       {0x09, 0x0E, 0x0B, 0x0D},
                       {0x0D, 0x09, 0x0E, 0x0B},
                       {0x0B, 0x0D, 0x09, 0x0E}};  //使用列混合矩阵的逆矩阵

    /* copy state[4][4] to tmp[4][4] */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j){
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = GMul(M[i][0], tmp[0][j]) ^ GMul(M[i][1], tmp[1][j])
                          ^ GMul(M[i][2], tmp[2][j]) ^ GMul(M[i][3], tmp[3][j]);
        }
    }

    return 0;
}


void __encrypData(uint8_t (*state)[4],const uint8_t *pt,uint8_t*ct,const uint32_t *rk)
{
    int j=0;
    // 把16字节的明文转换为4x4状态矩阵来进行处理
    loadStateArray(state, pt);
    // 轮秘钥加
    addRoundKey(state, rk);

    for (j = 1; j < 10; ++j) {
        rk += 4;
        subBytes(state);   // 字节替换
        shiftRows(state);  // 行移位
        mixColumns(state); // 列混合
        addRoundKey(state, rk); // 轮秘钥加
    }

    subBytes(state);    // 字节替换
    shiftRows(state);  // 行移位
    // 此处不进行列混合
    addRoundKey(state, rk+4); // 轮秘钥加

    // 把4x4状态矩阵转换为uint8_t一维数组输出保存
    storeStateArray(state, ct);
}


/**
 * @brief aesEncrypt 加密数据
 * @param key      密码
 * @param keyLen   密码长度，不能超过，小于16个字节，则补充0
 * @param pplaindata  被加密的数据
 * @param pdlen       被加密的数据长度
 * @param ppciphterdata   加密后的数据存放的位置指针，空间由函数内部分配，外部调用者使用之后用free释放
 * @param pcdlen      加密的数据长度存放位置
 * @return
 *         0表示成功，ppciphterdata和pcdlen可用，否则不可用
 *         负数表示失败
 */
int32_t aesEncrypt(const uint8_t *key,int32_t keyLen, const uint8_t *pplaindata,int32_t pdlen, uint8_t **ppciphterdata, int32_t*pcdlen)
{
    int32_t Ret=0;
    AesKey aesKey;
    int32_t i,ecryptDataSize;
    uint8_t *pTmpCipherData= NULL;
    const uint32_t *rk = aesKey.eK;  //解密秘钥指针
    uint8_t ResetV=0;
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};
    uint8_t TmpencryptData[BLOCKSIZE];
    int32_t LoopCnt,BytesLeft;

    if(ppciphterdata==NULL || pcdlen==NULL || pplaindata==NULL || pdlen==0){
        Ret=-1; goto __end;
    }

    LoopCnt = pdlen/BLOCKSIZE;
    BytesLeft = pdlen%BLOCKSIZE;
    ecryptDataSize=(pdlen+BLOCKSIZE-1)/BLOCKSIZE*BLOCKSIZE;
    if(BytesLeft==0){
        ecryptDataSize+=BLOCKSIZE;//向上取整之后再加一个BlockSize
    }
    *ppciphterdata = (uint8_t*)malloc(ecryptDataSize);
    if(*ppciphterdata==NULL){
        Ret=-1; goto __end;
    }
    pTmpCipherData=*ppciphterdata;
    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  // 秘钥扩展

    for(i=0;i<LoopCnt;i++){
        __encrypData(state,pplaindata,pTmpCipherData,rk);
        pTmpCipherData += BLOCKSIZE;  // 加密数据内存指针移动到下一个分组
        pplaindata += BLOCKSIZE;   // 明文数据指针移动到下一个分组
        rk = aesKey.eK;    // 恢复rk指针到秘钥初始位置
    }

    //pkcs7填充就是是如果不是16字节对齐的，那么缺多少字节，就填充几个几，（比如缺5个字节，补5个05， 那么填充05 05 05 05 05）
    //如果刚好是整数倍，则填充16个0x10
    ResetV=BLOCKSIZE-BytesLeft;
    memset(TmpencryptData,ResetV,BLOCKSIZE);
    if(BytesLeft!=0){
        memcpy(TmpencryptData,pplaindata,BytesLeft);
    }
    __encrypData(state,TmpencryptData,pTmpCipherData,rk);
    pTmpCipherData+=BLOCKSIZE;

    *pcdlen=(uint32_t)(pTmpCipherData-*ppciphterdata);
__end:
    return Ret;
}

/**
 * @brief aesDecrypt 对密文进行解密
 * @param key      密码
 * @param keyLen   密码长度，不能超过，小于16个字节，则补充0
 * @param pcipherdata  密文
 * @param pctlen       密文长度
 * @param ppplaindata  明文存储的位置，由函数内部分配空间，外部使用之后需要使用free释放
 * @param pptlen       明文实际长度
 * @return
 *      0表示成功则ppplaindata和pptlen可以使用
 *      负数表示失败
 */
int32_t aesDecrypt(const uint8_t *key,int32_t keyLen, const uint8_t *pcipherdata,int32_t pctlen, uint8_t **ppplaindata, int32_t*pptlen)
{
    int32_t Ret=0,i=0,j=0;
    AesKey aesKey;
    uint8_t *pTmpRawData = NULL;
    uint8_t PadData=0;
    const uint32_t *rk = aesKey.dK;  //解密秘钥指针
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};

    if(pcipherdata==NULL || pctlen==0 || ppplaindata==NULL || pptlen==NULL){//参数错误
        Ret=-1; goto __end;
    }

    if(pctlen%BLOCKSIZE!=0 ){//参数错误
        Ret=-1; goto __end;
    }

    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  //秘钥扩展，同加密

    *ppplaindata=(uint8_t*)malloc(pctlen); //分配和密文一样长度的明文存储空间
    if(*ppplaindata==NULL){
        Ret=-1; goto __end;
    }
    pTmpRawData=*ppplaindata;
    for ( i= 0; i < pctlen; i += BLOCKSIZE) {
        // 把16字节的密文转换为4x4状态矩阵来进行处理
        loadStateArray(state, pcipherdata);
        // 轮秘钥加，同加密
        addRoundKey(state, rk);

        for (j = 1; j < 10; ++j) {
            rk += 4;
            invShiftRows(state);    // 逆行移位
            invSubBytes(state);     // 逆字节替换，这两步顺序可以颠倒
            addRoundKey(state, rk); // 轮秘钥加，同加密
            invMixColumns(state);   // 逆列混合
        }

        invSubBytes(state);   // 逆字节替换
        invShiftRows(state);  // 逆行移位
        // 此处没有逆列混合
        addRoundKey(state, rk+4);  // 轮秘钥加，同加密

        storeStateArray(state, pTmpRawData);  // 保存明文数据
        pTmpRawData += BLOCKSIZE;  // 输出数据内存指针移位分组长度
        pcipherdata += BLOCKSIZE;   // 输入数据内存指针移位分组长度
        rk = aesKey.dK;    // 恢复rk指针到秘钥初始位置
    }

    PadData=*(*ppplaindata+pctlen-1);//确认Pad数据，得到加密时补充的长度
    *pptlen=pctlen-(int32_t)PadData; //实际长度需要用总长度减去Pad的长度

__end:
    return Ret;
}

/**
 * @brief aesEncryptUseOutBuff 加密数据
 * @param key      密码
 * @param keyLen   密码长度，不能超过，小于16个字节，则补充0
 * @param pplaindata  被加密的数据
 * @param pdlen       被加密的数据长度
 * @param ppciphterdata   加密数据存放的位置，由外部分配，pdlen向上16取整并,如果刚好为16的整数倍则要加16
 * @param cdlen      加密数据存放的位置允许使用的长度，pdlen向上16取整并,如果刚好为16的整数倍则要加16
 * @return
 *         大于0返回实际密文的长度，ppciphterdata可用，否则不可用
 *         <=0表示失败
 */
int32_t aesEncryptUseOutBuff(const uint8_t *key,int32_t keyLen, const uint8_t *pplaindata,int32_t pdlen, uint8_t *pciphterdata, int32_t cdlen)
{
    int32_t Ret=0;
    AesKey aesKey;
    int32_t i,ecryptDataSize;
    uint8_t *pTmpCipherData= NULL;
    const uint32_t *rk = aesKey.eK;  //解密秘钥指针
    uint8_t ResetV=0;
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};
    uint8_t TmpencryptData[BLOCKSIZE];
    int32_t LoopCnt,BytesLeft;

    if(pciphterdata==NULL || pplaindata==NULL || pdlen==0){
        Ret=-1; goto __end;
    }

    LoopCnt = pdlen/BLOCKSIZE;
    BytesLeft = pdlen%BLOCKSIZE;

    ecryptDataSize=(pdlen+BLOCKSIZE-1)/BLOCKSIZE*BLOCKSIZE;
    if(BytesLeft==0){
        ecryptDataSize+=BLOCKSIZE;//向上取整之后再加一个BlockSize
    }

    if(cdlen<ecryptDataSize){
        Ret=-1; goto __end;
    }

    pTmpCipherData=pciphterdata;
    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  // 秘钥扩展

    for(i=0;i<LoopCnt;i++){
        __encrypData(state,pplaindata,pTmpCipherData,rk);
        pTmpCipherData += BLOCKSIZE;  // 加密数据内存指针移动到下一个分组
        pplaindata += BLOCKSIZE;   // 明文数据指针移动到下一个分组
        rk = aesKey.eK;    // 恢复rk指针到秘钥初始位置
    }

    //pkcs7填充就是是如果不是16字节对齐的，那么缺多少字节，就填充几个几，（比如缺5个字节，补5个05， 那么填充05 05 05 05 05）
    //如果刚好是整数倍，则填充16个0x10
    ResetV=BLOCKSIZE-BytesLeft;
    memset(TmpencryptData,ResetV,BLOCKSIZE);
    if(BytesLeft!=0){
        memcpy(TmpencryptData,pplaindata,BytesLeft);
    }
    __encrypData(state,TmpencryptData,pTmpCipherData,rk);
    pTmpCipherData+=BLOCKSIZE;

    Ret=(int32_t)(pTmpCipherData-pciphterdata);
__end:
    return Ret;
}


/**
 * @brief aesDecryptUseOutBuff 对密文进行解密
 * @param key      密码
 * @param keyLen   密码长度，不能超过，小于16个字节，则补充0
 * @param pcipherdata  密文
 * @param pctlen       密文长度
 * @param pplaindata  明文存储的位置，由外部调用者进行分配，至少要和密文长度一样长
 * @param ptlen       明文存储的位置允许使用长度
 * @return
 *      >0表示明文实际使用的长度pplaindata可用
 *      <=0表示失败
 */
int32_t aesDecryptUseOutBuff(const uint8_t *key,int32_t keyLen, const uint8_t *pcipherdata,int32_t pctlen, uint8_t *pplaindata, int32_t ptlen)
{
    int32_t Ret=0,i=0,j=0;
    AesKey aesKey;
    uint8_t *pTmpRawData = NULL;
    uint8_t PadData=0;
    const uint32_t *rk = aesKey.dK;  //解密秘钥指针
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};

    if(pcipherdata==NULL || pctlen==0 || pplaindata==NULL){//参数错误
        Ret=-1; goto __end;
    }

    if(pctlen%BLOCKSIZE!=0 ){//参数错误
        Ret=-1; goto __end;
    }

    if(ptlen<pctlen){
        Ret=-1; goto __end;
    }

    memcpy(actualKey, key, keyLen);
    keyExpansion(actualKey, 16, &aesKey);  //秘钥扩展，同加密

    pTmpRawData=pplaindata;
    for ( i= 0; i < pctlen; i += BLOCKSIZE) {
        // 把16字节的密文转换为4x4状态矩阵来进行处理
        loadStateArray(state, pcipherdata);
        // 轮秘钥加，同加密
        addRoundKey(state, rk);

        for (j = 1; j < 10; ++j) {
            rk += 4;
            invShiftRows(state);    // 逆行移位
            invSubBytes(state);     // 逆字节替换，这两步顺序可以颠倒
            addRoundKey(state, rk); // 轮秘钥加，同加密
            invMixColumns(state);   // 逆列混合
        }

        invSubBytes(state);   // 逆字节替换
        invShiftRows(state);  // 逆行移位
        // 此处没有逆列混合
        addRoundKey(state, rk+4);  // 轮秘钥加，同加密

        storeStateArray(state, pTmpRawData);  // 保存明文数据
        pTmpRawData += BLOCKSIZE;  // 输出数据内存指针移位分组长度
        pcipherdata += BLOCKSIZE;   // 输入数据内存指针移位分组长度
        rk = aesKey.dK;    // 恢复rk指针到秘钥初始位置
    }

    PadData=*(pplaindata+pctlen-1);//确认Pad数据，得到加密时补充的长度
    Ret=pctlen-(int32_t)PadData; //实际长度需要用总长度减去Pad的长度

__end:
    return Ret;
}


