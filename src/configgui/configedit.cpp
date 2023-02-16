#include "configedit.h"
#include "clogclient.h"
#include "cconfigclient.h"
#include "ctools.h"
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <string>
#include <fstream>

static ConfigEdit *curEdit = nullptr;

static void configMessageCb(bool isOk, const char *msg) {
  if (curEdit) {
    curEdit->messageCbSig(isOk, msg);
  }
}

ConfigEdit::ConfigEdit(QWidget *parent) : QDialog(parent) {
  curEdit = this;
  ui.setupUi(this);
  setLayout(ui.formLayout);
  // �����źŲ�
  QObject::connect(this, SIGNAL(messageCbSig(bool, const char *)), this, SLOT(messageCb(bool, const char *)));
  // �趨�����ϴ����ú�����Ϣ�Ļص������������жϽ����Ƿ�Ӧ��ˢ��
  CConfigClient::get()->uploadConfigResCb_ = configMessageCb;
  // ��¼���õĻ�����Ϣ����
  configRowCount_ = ui.formLayout->rowCount();
  if (!config_) {
    config_ = new cmsg::CConfig();
  }
}

ConfigEdit::~ConfigEdit() {
  curEdit = nullptr;
  if (config_) {
    delete config_;
    config_ = nullptr;
  }
}

void ConfigEdit::messageCb(bool isOk, const char *msg) {
  if (!isOk) {
    QMessageBox::information(this, "", msg);
    return;
  }
  accept();
}

void ConfigEdit::save() {
  if (!message_) {
    LOG_DEBUG("save failed: message_ is nullptr!");
    QMessageBox::information(this, "", "proto file not set!");
    return;
  }

  if (ui.serviceNameLineEdit->text().isEmpty()
      || ui.serviceIpLineEdit->text().isEmpty()
      || ui.protoTextEdit->toPlainText().isEmpty()) {
    QMessageBox::information(this, "", "make sure serviceName/serviceIp/protoType has valid value!");
    return;
  }
  
  // �������� ���� ������Ϣ(���� CConfig) �� ������Ϣ
  // ������Ϣ������������洢�� message ��
  // ��ȡ��������
  auto desc = message_->GetDescriptor();
  // message ����
  auto ref = message_->GetReflection();
  // ��������
  for (int i = configRowCount_; i < ui.formLayout->rowCount(); ++i) {
    // �� label �� text ���ҵ� key 
    auto labelItem = ui.formLayout->itemAt(i, QFormLayout::LabelRole);
    if (!labelItem) {
      continue;
    }
    // ����ʱת����ʧ�ܷ��� nullptr
    auto label = dynamic_cast<QLabel *>(labelItem->widget());
    if (!label) {
      continue;
    }
    // �� label �л�ȡ���� key
    auto fieldName = label->text().toStdString();

    // ��ȡ value������ؼ��е�ֵ��ö�١����Ρ����㡢�ַ���
    //  ��ȡ�ؼ�
    auto fieldItem = ui.formLayout->itemAt(i, QFormLayout::FieldRole);
    if (!fieldItem) {
      continue;
    }
    auto fieldEdit = fieldItem->widget();
    //  ��ȡ�ֶ�������(����)
    auto fieldDesc = desc->FindFieldByName(fieldName);
    if (!fieldDesc) {
      continue;
    }
    auto type = fieldDesc->type();
    // ��ȡ�ؼ���ֵ�����õ� message
    QSpinBox *intBox = nullptr;           // ����
    QDoubleSpinBox *doubleBox = nullptr;  // ����
    QLineEdit *strEdit = nullptr;         // �ַ����� byte
    QComboBox *comboBox = nullptr;        // ������ö��
    // ��ȡ�ؼ���ֵ�����õ� message (����)
    switch (type) {
      case google::protobuf::FieldDescriptor::TYPE_INT64:
        intBox = dynamic_cast<QSpinBox *>(fieldEdit);
        if (!intBox) continue;
        ref->SetInt64(message_, fieldDesc, intBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_INT32:
        intBox = dynamic_cast<QSpinBox *>(fieldEdit);
        if (!intBox) continue;
        ref->SetInt32(message_, fieldDesc, intBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
        doubleBox = dynamic_cast<QDoubleSpinBox *>(fieldEdit);
        if (!doubleBox) continue;
        ref->SetDouble(message_, fieldDesc, doubleBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_FLOAT:
        doubleBox = dynamic_cast<QDoubleSpinBox *>(fieldEdit);
        if (!doubleBox) continue;
        ref->SetFloat(message_, fieldDesc, doubleBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_BYTES:
      case google::protobuf::FieldDescriptor::TYPE_STRING:
        strEdit = dynamic_cast<QLineEdit *>(fieldEdit);
        if (!strEdit) continue;
        ref->SetString(message_, fieldDesc, strEdit->text().toStdString());
        break;
      case google::protobuf::FieldDescriptor::TYPE_BOOL:
        comboBox = dynamic_cast<QComboBox *>(fieldEdit);
        if (!comboBox) continue;
        ref->SetBool(message_, fieldDesc, comboBox->currentData().toBool());
        break;
      case google::protobuf::FieldDescriptor::TYPE_ENUM:
        comboBox = dynamic_cast<QComboBox *>(fieldEdit);
        if (!comboBox) continue;
        ref->SetEnumValue(message_, fieldDesc, comboBox->currentData().toInt());
        break;
      default:
        break;
    }
  }

  // ������Ϣ
  cmsg::CConfig config;
  config.set_servicename(ui.serviceNameLineEdit->text().toStdString());
  config.set_serviceip(ui.serviceIpLineEdit->text().toStdString());
  config.set_serviceport(ui.servicePortSpinBox->value());
  config.set_proto(ui.protoTextEdit->toPlainText().toStdString());
  
  // ���л� message
  std::string msgPb = message_->SerializeAsString();
  config.set_privatepb(msgPb);

  LOG_DEBUG(message_->DebugString());
  LOG_DEBUG(config.DebugString());
  // �ϴ����õ���������
  CConfigClient::get()->uploadConfig(&config);
}

void ConfigEdit::loadProto() {
  LOG_DEBUG("load Proto");

  // �û������������ƣ����û�����ƣ���ʹ�� proto �ļ��еĵ�һ������
  QString className = ui.typeLineEdit->text();
  std::string classNameStr = "";
  if (!className.isEmpty()) {
    classNameStr = className.toStdString();
  }

  // �û�ѡ�� proto �ļ�
  QString filename = QFileDialog::getOpenFileName(this,
    QString::fromLocal8Bit("��ѡ�� proto �ļ�"), "", "*.proto");
  if (filename.isEmpty()) {
    return;
  }

  //LOG_DEBUG(filename.toStdString().c_str());

  // ��ȡ����� message ����
  std::string protoCode = "";
  message_ = CConfigClient::get()->loadProto(filename.toStdString(), classNameStr, protoCode);
  if (!message_) {
    LOG_DEBUG("CConfigClient::get()->loadProto failed!")
    return;
  }
  config_->set_proto(protoCode);
  initGui();
}

bool ConfigEdit::loadConfig(const char *ip, int port) {
  // ������Ϣ��ȡ������ CConfig���洢����Ա config_ ��
  CConfigClient::get()->downloadConfig(ip, port);
  if (!config_)
    config_ = new cmsg::CConfig();
  bool isGet = false;

  // ��ʱ�ȴ�һ��
  for (int i = 0; i < 100; ++i) {
    if (CConfigClient::get()->getConfig(ip, port, config_)) {
      isGet = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (!isGet) {
    LOG_DEBUG("��ȡ��Ҫ�޸ĵ���������ʧ��!");
    return false;
  }
  LOG_DEBUG("��ȡ��Ҫ�޸ĵ��������ݳɹ�!");
  if (config_->proto().empty()) {
    LOG_DEBUG("���õ� proto Ϊ��!");
    return false;
  }
  
  // ��������д�� proto �ļ�
  std::string protoFilePath = "tmp.proto";
  std::ofstream ofs(protoFilePath);
  ofs << config_->proto();
  ofs.close();

  // ���� proto������ message
  std::string outProto = "";
  // ȡ proto �ļ��еĵ�һ������
  message_ = CConfigClient::get()->loadProto(protoFilePath, "", outProto);
  if (!message_) {
    LOG_DEBUG("���� proto �ļ�ʧ��!");
    return false;
  }
  //LOG_DEBUG(config_->DebugString());
  //LOG_DEBUG(message_->GetDescriptor()->DebugString());

  // message �����л������� proto �ļ�������
  if (!message_->ParseFromString(config_->privatepb())) {
    LOG_DEBUG("�����л� message ʧ��!");
    return false;
  }
  LOG_DEBUG(message_->DebugString().c_str());

  initGui();
  return true;
}

void ConfigEdit::initGui() {
  // ������֮ǰ��¼��������Ϣ��
  while (ui.formLayout->rowCount() != configRowCount_) {
    ui.formLayout->removeRow(configRowCount_);
  }

  // �������õĻ�����Ϣ
  if (config_) {
    ui.serviceIpLineEdit->setText(config_->serviceip().c_str());
    ui.serviceNameLineEdit->setText(config_->servicename().c_str());
    ui.servicePortSpinBox->setValue(config_->serviceport());
    ui.protoTextEdit->setText(config_->proto().c_str());
  }

  if (!message_) {
    return;
  }

  ui.typeLineEdit->setText(message_->GetTypeName().c_str());

  // ͨ���������� message ���棬���趨ֵ
  // ��ȡ��������
  auto desc = message_->GetDescriptor();
  // ͨ�������趨�����ֵ
  auto ref = message_->GetReflection();
  // �����ֶ�
  int fieldCount = desc->field_count();
  for (int i = 0; i < fieldCount; ++i) {
    // �����ֶ�����
    auto field = desc->field(i);
    auto type = field->type();
    // ֧��:���֡��ַ�����ö��
    QSpinBox *intBox = nullptr;           // ����
    QDoubleSpinBox *doubleBox = nullptr;  // ����
    QLineEdit *strEdit = nullptr;         // �ַ����� byte
    QComboBox *comboBox = nullptr;        // ������ö��
    switch (type) {
      // ֧����������
    case google::protobuf::FieldDescriptor::TYPE_INT64:
      intBox = new QSpinBox();
      intBox->setMaximum(INT64_MAX);
      intBox->setValue(ref->GetInt64(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), intBox);
      break;
    case google::protobuf::FieldDescriptor::TYPE_INT32:
      intBox = new QSpinBox();
      intBox->setMaximum(INT32_MAX);
      intBox->setValue(ref->GetInt32(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), intBox);
      break;
      // ֧�ָ�����
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
      doubleBox = new QDoubleSpinBox();
      doubleBox->setMaximum(DBL_MAX);
      doubleBox->setValue(ref->GetDouble(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), doubleBox);
      break;
    case google::protobuf::FieldDescriptor::TYPE_FLOAT:
      doubleBox = new QDoubleSpinBox();
      doubleBox->setMaximum(FLT_MAX);
      doubleBox->setValue(ref->GetFloat(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), doubleBox);
      break;
      // ֧���ַ����� byte
    case google::protobuf::FieldDescriptor::TYPE_BYTES:
    case google::protobuf::FieldDescriptor::TYPE_STRING:
      strEdit = new QLineEdit();
      strEdit->setText(ref->GetString(*message_, field).c_str());
      ui.formLayout->addRow(field->name().c_str(), strEdit);
      break;
      // ֧�� bool ����
    case google::protobuf::FieldDescriptor::TYPE_BOOL:
      comboBox = new QComboBox();
      comboBox->addItem("true", true);
      comboBox->addItem("false", false);
      if (ref->GetBool(*message_, field)) {
        comboBox->setCurrentIndex(0);
      } else {
        comboBox->setCurrentIndex(1);
      }
      ui.formLayout->addRow(field->name().c_str(), comboBox);
      break;
      // ֧��ö������
    case google::protobuf::FieldDescriptor::TYPE_ENUM:
      comboBox = new QComboBox();
      for (int j = 0; j < field->enum_type()->value_count(); ++j) {
        // ��ȡö����
        std::string enumName = field->enum_type()->value(j)->name();
        // ��ȡ��ö������Ӧ��ֵ
        int enumVal = field->enum_type()->value(j)->number();
        // �����ö��ѡ��
        comboBox->addItem(enumName.c_str(), enumVal);
      }
      ui.formLayout->addRow(field->name().c_str(), comboBox);
      // ���ݶ�Ӧ�� index ��ȡֵ
      comboBox->setCurrentIndex(comboBox->findData(ref->GetEnumValue(*message_, field)));
      break;
    default:
      break;
    }
  }
}

