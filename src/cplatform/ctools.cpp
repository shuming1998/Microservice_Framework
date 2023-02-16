#include "ctools.h"
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif // !_WIN32


CCOME_API std::string getDirData(std::string path) {
  std::string data = "";
#ifdef _WIN32
  // �洢�ļ���Ϣ
  _finddata_t file;
  std::string dirPath = path + "/*.*";
  // Ŀ¼������
  intptr_t dir = _findfirst(dirPath.c_str(), &file);
  if (dir < 0) {
    return data;
  }
  do {
    if (file.attrib & _A_SUBDIR) {
      continue;
    }
    char buf[1024] = { 0 };
    sprintf(buf, "%s,%u;", file.name, file.size);
    data += buf;
  } while (_findnext(dir, &file) == 0);
#else
  const char *dir = path.c_str();
  DIR *dp = 0;
  struct dirent *entry = 0;
  struct stat statBuf;
  dp = opendir(dir);
  if (!dp) {
    return data;
  }
  chdir(dir);
  char buf[1024] = { 0 };
  while ((entry = readdir(dp)) != NULL) {
    lstat(entry->d_name, &statBuf);
    if (S_ISDIR(statBuf.st_mode)) {
      continue;
    }
    sprintf(buf, "%s,%ld;", entry->d_name, statBuf.st_size);
    data += buf;
  }
#endif // _WIN32
  // ȥ����β�� ';'
  if (!data.empty()) {
    data = data.substr(0, data.size() - 1);
  }
  return data;
}


CCOME_API int base64Encode(const unsigned char *in, int len, char *outBase64) {
  if (!in || len <= 0 || !outBase64) {
    return 0;
  }
  // �����ڴ�Դ�����ڴ洢���
  auto memBio = BIO_new(BIO_s_mem());
  if (!memBio) {
    return 0;
  }

  // base64 ������
  auto b64Bio = BIO_new(BIO_f_base64());
  if (!b64Bio) {
    BIO_free(memBio);
    return 0;
  }

  // ���� 64 �ֽڲ��ӻ��з�
  BIO_set_flags(b64Bio, BIO_FLAGS_BASE64_NO_NL);

  // �γ� BIO ���� : b64Bio--memBio
  BIO_push(b64Bio, memBio);

  // ������ͷ��д�룬base64�����������ת����һ���ڵ�(��β)
  // ������д�����ñ���
  int res = BIO_write(b64Bio, in, len);
  if (res <= 0) {
    // ��������
    BIO_free_all(b64Bio);
    return 0;
  }

  // ˢ�»��棬д������� mem
  BIO_flush(b64Bio);

  int outSize = 0;
  BUF_MEM *pData = nullptr;
  // ���������Դ�ڴ� memBio �ж�ȡ
  BIO_get_mem_ptr(b64Bio, &pData);
  if (pData) {
    memcpy(outBase64, pData->data, pData->length);
    outSize = pData->length;
  }
  BIO_free_all(b64Bio);
  return outSize;
}

CCOME_API int base64Decode(const char *in, int len, unsigned char *outData) {
  if (!in || len <= 0 || !outData) {
    return 0;
  }
  // �����ڴ�Դ������������������
  auto memBio = BIO_new_mem_buf(in, len);
  if (!memBio) {
    return 0;
  }

  // base64 ������
  auto b64Bio = BIO_new(BIO_f_base64());
  if (!b64Bio) {
    BIO_free(memBio);
    return 0;
  }

  // ���� 64 �ֽڲ��ӻ��з�
  BIO_set_flags(b64Bio, BIO_FLAGS_BASE64_NO_NL);

  // �γ� BIO ���� : b64Bio--memBio
  BIO_push(b64Bio, memBio);

  // ��ȡ����������
  size_t size = 0;
  BIO_read_ex(b64Bio, outData, len, &size);

  BIO_free_all(b64Bio);
  return size;
}