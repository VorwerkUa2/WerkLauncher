/* Copyright 2025 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "minecraft/auth/AuthStep.h"
#include "skins/SkinTypes.h"
#include <QObject>


class YggdrasilLoginStep : public AuthStep {
  Q_OBJECT
public:
  explicit YggdrasilLoginStep(AccountData *data);
  virtual ~YggdrasilLoginStep() noexcept;

  void perform() override;
  QString describe() override;

private slots:
  void onRequestDone(QNetworkReply::NetworkError, QByteArray,
                     QList<QNetworkReply::RawHeaderPair>);
};

class YggdrasilSetSkinStep : public AuthStep {
  Q_OBJECT
public:
  explicit YggdrasilSetSkinStep(AccountData *data, Skins::Model model,
                                QByteArray skinData);
  virtual ~YggdrasilSetSkinStep() noexcept;

  void perform() override;
  QString describe() override;

private slots:
  void onRequestDone(QNetworkReply::NetworkError, QByteArray,
                     QList<QNetworkReply::RawHeaderPair>);

private:
  Skins::Model m_model;
  QByteArray m_skinData;
};
