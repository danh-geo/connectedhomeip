/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// THIS FILE IS GENERATED BY ZAP

#include "CHIPClusters.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>

namespace chip {

using namespace app::Clusters;
using namespace System;
using namespace Encoding::LittleEndian;

namespace Controller {

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::Move::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::Move::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type,
                                               chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::MoveWithOnOff::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::Step::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::Step::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::StepWithOnOff::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::Stop::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::Stop::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::LevelControl::Commands::StopWithOnOff::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::LevelControl::Commands::StopWithOnOff::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::OnOff::Commands::Off::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::OnOff::Commands::Off::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::OnOff::Commands::On::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::OnOff::Commands::On::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template CHIP_ERROR
ClusterBase::InvokeCommand<chip::app::Clusters::OnOff::Commands::Toggle::Type, chip::app::DataModel::NullObjectType>(
    const chip::app::Clusters::OnOff::Commands::Toggle::Type &, void *,
    CommandResponseSuccessCallback<chip::app::DataModel::NullObjectType>, CommandResponseFailureCallback);

template <typename RequestDataT, typename ResponseDataT>
CHIP_ERROR ClusterBase::InvokeCommand(const RequestDataT & requestData, void * context,
                                      CommandResponseSuccessCallback<ResponseDataT> successCb,
                                      CommandResponseFailureCallback failureCb)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mDevice->LoadSecureSessionParametersIfNeeded());

    auto onSuccessCb = [context, successCb](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                            const ResponseDataT & responseData) { successCb(context, responseData); };

    auto onFailureCb = [context, failureCb](const app::StatusIB & aStatus, CHIP_ERROR aError) {
        failureCb(context, app::ToEmberAfStatus(aStatus.mStatus));
    };

    return InvokeCommandRequest<ResponseDataT>(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(), mEndpoint,
                                               requestData, onSuccessCb, onFailureCb);
};

} // namespace Controller
} // namespace chip