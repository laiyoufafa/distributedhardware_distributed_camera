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

#ifndef DISTRIBUTED_CAMERA_HOST_CALLBACK_PROXY_H
#define DISTRIBUTED_CAMERA_HOST_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "icamera_host_callback.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Camera;
class DCameraHostCallbackProxy : public IRemoteProxy<ICameraHostCallback> {
public:
    explicit DCameraHostCallbackProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ICameraHostCallback>(impl) {}

    virtual ~DCameraHostCallbackProxy() = default;

    virtual void OnCameraStatus(const std::string &cameraId, CameraStatus status);
    virtual void OnFlashlightStatus(const std::string &cameraId, FlashlightStatus status);
    virtual void OnCameraEvent(const std::string &cameraId, CameraEvent event);

private:
    static inline BrokerDelegator<DCameraHostCallbackProxy> delegator_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTED_CAMERA_HOST_CALLBACK_PROXY_H
