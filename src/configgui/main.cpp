#include "configgui.h"
#include "cconfigclient.h"
#include "configedit.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[]) {
  // ��ʼ�����ÿͻ��ˣ������̳߳�
  CConfigClient::get()->startGetConfig("127.0.0.1", CONFIG_PORT, 0, 0, 0);
  QApplication a(argc, argv);
  //ConfigEdit edit;
  //edit.exec();
  //return 0;
  ConfigGui w;
  w.show();
  return a.exec();
}
