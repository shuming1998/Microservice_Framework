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

//#define LOG_DEBUG(msg) cms::cLog(cmsg::CLOG_DEBUG, msg, __FILE__, __LINE__);
//#define LOG_INFO(msg) cms::cLog(cmsg::CLOG_INFO, msg, __FILE__, __LINE__);
//#define LOG_ERROR(msg) cms::cLog(cmsg::CLOG_ERROR, msg, __FILE__, __LINE__);
//#define LOG_FATAL(msg) cms::cLog(cmsg::CLOG_FATAL, msg, __FILE__, __LINE__);

//#define LOG(level, msg) std::cout << '[' <<  level << ']' << __FILE__ << ':' << __LINE__ << '\n' << msg << '\n';
//#define LOG_INFO(msg) LOG("INFO", msg);
//#define LOG_ERROR(msg) LOG("ERROR", msg);
//#define LOG_DEBUG(msg) LOG("DEBUG", msg);

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





/* fmt
%a ���ڼ��ļ�д
%A ���ڼ���ȫ��
%b �·ֵļ�д
%B �·ݵ�ȫ��
%c ��׼�����ڵ�ʱ�䴮
%C ��ݵĺ���λ����
%d ʮ���Ʊ�ʾ��ÿ�µĵڼ���
%D ��/��/��
%e �����ַ����У�ʮ���Ʊ�ʾ��ÿ�µĵڼ���
%F ��-��-��
%g ��ݵĺ���λ���֣�ʹ�û����ܵ���
%G ��֣�ʹ�û����ܵ���
%h ��д���·���
%H 24Сʱ�Ƶ�Сʱ
%I 12Сʱ�Ƶ�Сʱ
%j ʮ���Ʊ�ʾ��ÿ��ĵڼ���
%m ʮ���Ʊ�ʾ���·�
%M ʮʱ�Ʊ�ʾ�ķ�����
%n ���з�
%p ���ص�AM��PM�ĵȼ���ʾ
%r 12Сʱ��ʱ��
%R ��ʾСʱ�ͷ��ӣ�hh:mm
%S ʮ���Ƶ�����
%t ˮƽ�Ʊ��
%T ��ʾʱ���룺hh:mm:ss
%u ÿ�ܵĵڼ��죬����һΪ��һ�� ��ֵ��0��6������һΪ0��
%U ����ĵڼ��ܣ�����������Ϊ��һ�죨ֵ��0��53��
%V ÿ��ĵڼ��ܣ�ʹ�û����ܵ���
%w ʮ���Ʊ�ʾ�����ڼ���ֵ��0��6��������Ϊ0��
%W ÿ��ĵڼ��ܣ�������һ��Ϊ��һ�죨ֵ��0��53��
%x ��׼�����ڴ�
%X ��׼��ʱ�䴮
%y �������͵�ʮ������ݣ�ֵ��0��99��
%Y �����Ͳ��ֵ�ʮ�����
%z��%Z ʱ�����ƣ�������ܵõ�ʱ�������򷵻ؿ��ַ���
%% �ٷֺ�����ĳ�������ʾ��ǰ���������ڣ�
*/
CCOME_API std::string cgetTime(int timestamp, std::string fmt = "%F %T");



#endif // !CTOOLS_H

