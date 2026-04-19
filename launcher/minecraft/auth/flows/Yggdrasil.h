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
#include "AuthFlow.h"
#include "skins/SkinTypes.h"
#include <QByteArray>
#include <QObject>
#include <QString>

class YggdrasilSetSkin : public AuthFlow {
  Q_OBJECT
public:
  explicit YggdrasilSetSkin(AccountData *data, const QByteArray &skinData,
                            Skins::Model model, QObject *parent = 0);
};

class YggdrasilLogin : public AuthFlow {
  Q_OBJECT
public:
  explicit YggdrasilLogin(AccountData *data, QObject *parent = 0);
};
