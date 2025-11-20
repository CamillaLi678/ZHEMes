#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>



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
int32_t aesEncrypt(const uint8_t *key,int32_t keyLen, const uint8_t *pplaindata,int32_t pdlen,
                    uint8_t **ppciphterdata, int32_t*pcdlen);

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
int32_t aesDecrypt(const uint8_t *key,int32_t keyLen, const uint8_t *pcipherdata,int32_t pctlen,
                   uint8_t **ppplaindata, int32_t*pptlen);


/**
 * @brief aesEncryptUseOutBuff 加密数据
 * @param key      密码
 * @param keyLen   密码长度，不能超过，小于16个字节，则补充0
 * @param pplaindata  被加密的数据
 * @param pdlen       被加密的数据长度
 * @param ppciphterdata   加密数据存放的位置，由外部分配
 * @param cdlen      加密数据存放的位置允许使用的长度，pdlen向上16取整并,如果刚好为16的整数倍则要加16
 * @return
 *         大于0返回实际密文的长度，ppciphterdata可用，否则不可用
 *         <=0表示失败
 */
int32_t aesEncryptUseOutBuff(const uint8_t *key,int32_t keyLen, const uint8_t *pplaindata,int32_t pdlen,
                             uint8_t *pciphterdata, int32_t cdlen);


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
int32_t aesDecryptUseOutBuff(const uint8_t *key,int32_t keyLen, const uint8_t *pcipherdata,int32_t pctlen,
                             uint8_t *pplaindata, int32_t ptlen);

#endif
