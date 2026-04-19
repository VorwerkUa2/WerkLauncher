#pragma once
#include "BaseInstance.h"
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class SidePanel : public QWidget {
  Q_OBJECT
public:
  explicit SidePanel(QWidget *parent = nullptr);
  void setInstance(InstancePtr inst);
  void toggle(bool visible);

protected:
  void paintEvent(QPaintEvent *event) override;
  void changeEvent(QEvent *event) override;

private:
  void setupUi();
  void updateStyles();

  InstancePtr m_instance;
  QLabel *m_iconLabel;
  QLabel *m_nameLabel;
  QLabel *m_versionLabel;
  QLabel *m_playTimeLabel;

  QPushButton *m_playButton;
  QPushButton *m_settingsButton;
  QPushButton *m_logButton;

  QPropertyAnimation *m_anim;
};
