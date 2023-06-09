/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_CAMERA_OFFLINE_STREAM_OPERATOR_H
#define DISTRIBUTED_CAMERA_OFFLINE_STREAM_OPERATOR_H

#include "doffline_stream_operator_stub.h"
#include "dcamera.h"

namespace OHOS {
namespace DistributedHardware {
class DOfflineStreamOperator : public DOfflineStreamOperatorStub {
public:
    DOfflineStreamOperator() = default;
    virtual ~DOfflineStreamOperator() = default;
    DOfflineStreamOperator(const DOfflineStreamOperator &other) = delete;
    DOfflineStreamOperator(DOfflineStreamOperator &&other) = delete;
    DOfflineStreamOperator& operator=(const DOfflineStreamOperator &other) = delete;
    DOfflineStreamOperator& operator=(DOfflineStreamOperator &&other) = delete;

public:
    virtual CamRetCode CancelCapture(int captureId) override;
    virtual CamRetCode ReleaseStreams(const std::vector<int>& streamIds) override;
    virtual CamRetCode Release() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTED_CAMERA_OFFLINE_STREAM_OPERATOR_H