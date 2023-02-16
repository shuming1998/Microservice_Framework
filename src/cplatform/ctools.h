#ifndef CTOOLS_H
#define CTOOLS_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

#include <string>
#include <string.h>
#include <sstream>
#include <iostream>

#define LOG(level, msg) std::cout << '[' <<  level << ']' << __FILE__ << ':' << __LINE__ << '\n' << msg << '\n';
#define LOG_INFO(msg) LOG("INFO", msg);
#define LOG_ERROR(msg) LOG("ERROR", msg);
#define LOG_DEBUG(msg) LOG("DEBUG", msg);

CCOME_API std::string getDirData(std::string path);

// ����������ݵĴ�С
CCOME_API int base64Encode(const unsigned char *in, int len, char *outBase64);
CCOME_API int base64Decode(const char *in, int len, unsigned char *outData);

///����md5 128bit(16�ֽ�) 
///@para in_data ��������
///@para in_data_size ���������ֽ���
///@para out_md �����MD5���� ��16�ֽڣ�
CCOME_API unsigned char *CMD5(const unsigned char *inData, unsigned long inDataSize, unsigned char *outMd);

///����md5Base64  (24�ֽ�) �پ���base64ת��Ϊ�ַ���
///@para inData ��������
///@para inDataSize ���������ֽ���
///@return  �����MD5 base64 ���� ��24�ֽڣ�
CCOME_API std::string CMD5Base64(const unsigned char *inData, unsigned long inDataSize);









#endif // !CTOOLS_H

