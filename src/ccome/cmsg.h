#ifndef CMSG_H
#define CMSG_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

constexpr int MSG_MAX_SIZE = 1024000;  // ��Ϣ����ֽ���

enum MsgType {
  MSG_NONE = 0,
  MSG_GET_DIR,            // �ͻ�������Ŀ¼
  MSG_UPLOAD_INFO,        // �ͻ��������ϴ��ļ�
  MSG_DOWNLOAD_INFO,      // �ͻ������������ļ�
  MSG_DOWNLOAD_COMPLETE,  // �ͻ��������ļ��ɹ�

  MSG_DIR_LIST,           // ����˷���Ŀ¼�б�
  MSG_UPLOAD_ACCEPT,      // �����׼���ý����ļ�
  MSG_UPLOAD_COMPLETE,    // ����˽����ļ����
  MSG_DOWNLOAD_ACCEPT,    // �����ȷ���ļ����Կ�ʼ����


  MSG_MAX_TYPE,           // �߽�ֵ������֤������ȷ
};

// ��Ϣͷ
struct CCOME_API CMsgHd {
  MsgType type_;
  int size_ = 0;
};

// ��Ϣ����
// Լ��ÿ����Ϣ����������ݣ�����ʹ�� OK
struct CCOME_API CMsg : public CMsgHd {
  char *data_ = 0;        // �洢��Ϣ����
  int recved_ = 0;        // �ѽ��յ���Ϣ�ֽ���
};

#endif
