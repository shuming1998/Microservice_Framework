#pragma once

#include <QtWidgets/QWidget>
#include "ui_configgui.h"

class ConfigGui : public QWidget
{
    Q_OBJECT

public:
    ConfigGui(QWidget *parent = nullptr);
    ~ConfigGui();
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

public slots:
  // ˢ����ʾ�����б�
  void refresh();
  // ��������
  void addConfig();
  // ɾ��ѡ�е�����
  void delConfig();
  // �༭ѡ�е�����
  void editConfig();

  // ��ʾ����־�б���
  void addLog(const char *log);
private:
    Ui::ConfigGuiClass ui;

};
