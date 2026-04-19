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

#include "Yggdrasil.h"
#include "minecraft/auth/steps/GetSkinStep.h"
#include "minecraft/auth/steps/YggdrasilProfileStep.h"
#include "minecraft/auth/steps/YggdrasilStep.h"

YggdrasilSetSkin::YggdrasilSetSkin(AccountData *data,
                                   const QByteArray &skinData,
                                   Skins::Model model, QObject *parent)
    : AuthFlow(data, parent) {
  m_steps.append(new YggdrasilLoginStep(m_data));
  {
    auto apiCall = new YggdrasilSetSkinStep(m_data, model, skinData);
    m_steps.append(apiCall);
    // connect(apiCall, &YggdrasilSetSkinStep::apiError, this,
    // &AccountTask::apiError);
  }
  m_steps.append(new YggdrasilProfileStep(m_data));
  m_steps.append(new GetSkinStep(m_data));
}

YggdrasilLogin::YggdrasilLogin(AccountData *data, QObject *parent)
    : AuthFlow(data, parent) {
  m_steps.append(new YggdrasilLoginStep(m_data));
  m_steps.append(new YggdrasilProfileStep(m_data));
  m_steps.append(new GetSkinStep(m_data));
}
