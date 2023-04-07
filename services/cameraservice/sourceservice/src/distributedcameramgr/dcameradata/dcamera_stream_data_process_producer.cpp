/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dcamera_stream_data_process_producer.h"

#include <chrono>
#include <securec.h>

#include "anonymous_string.h"
#include "dcamera_buffer_handle.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include <sys/prctl.h>
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {
DCameraStreamDataProcessProducer::DCameraStreamDataProcessProducer(std::string devId, std::string dhId,
    int32_t streamId, DCStreamType streamType)
    : devId_(devId), dhId_(dhId), streamId_(streamId), streamType_(streamType), eventHandler_(nullptr),
    camHdiProvider_(nullptr)
{
    DHLOGI("DCameraStreamDataProcessProducer Constructor devId %s dhId %s streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    state_ = DCAMERA_PRODUCER_STATE_STOP;
    interval_ = DCAMERA_PRODUCER_ONE_MINUTE_MS / DCAMERA_PRODUCER_FPS_DEFAULT;
}

DCameraStreamDataProcessProducer::~DCameraStreamDataProcessProducer()
{
    DHLOGI("DCameraStreamDataProcessProducer Destructor devId %s dhId %s state: %d streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), state_, streamType_, streamId_);
    if (state_ == DCAMERA_PRODUCER_STATE_START) {
        Stop();
    }
}

void DCameraStreamDataProcessProducer::Start()
{
    DHLOGI("DCameraStreamDataProcessProducer Start producer devId: %s dhId: %s streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider_ == nullptr) {
        DHLOGE("camHdiProvider_ is null.");
    }
    state_ = DCAMERA_PRODUCER_STATE_START;
    if (streamType_ == CONTINUOUS_FRAME) {
        eventThread_ = std::thread(&DCameraStreamDataProcessProducer::StartEvent, this);
        std::unique_lock<std::mutex> lock(eventMutex_);
        eventCon_.wait(lock, [this] {
            return eventHandler_ != nullptr;
        });
        smoother_ = std::make_unique<DCameraFeedingSmoother>();
        smootherListener_ = std::make_shared<FeedingSmootherListener>(shared_from_this());
        smoother_->RegisterListener(smootherListener_);
        smoother_->StartSmooth();
    } else {
        producerThread_ = std::thread(&DCameraStreamDataProcessProducer::LooperSnapShot, this);
    }
}

void DCameraStreamDataProcessProducer::Stop()
{
    DHLOGI("DCameraStreamDataProcessProducer Stop devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        state_ = DCAMERA_PRODUCER_STATE_STOP;
    }
    producerCon_.notify_one();
    if (streamType_ == CONTINUOUS_FRAME) {
        smoother_->StopSmooth();
        smoother_ = nullptr;
        smootherListener_ = nullptr;
        eventHandler_->GetEventRunner()->Stop();
        eventThread_.join();
        eventHandler_ = nullptr;
    } else {
        producerThread_.join();
    }
    camHdiProvider_ = nullptr;
    DHLOGI("DCameraStreamDataProcessProducer Stop end devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

void DCameraStreamDataProcessProducer::FeedStream(const std::shared_ptr<DataBuffer>& buffer)
{
    buffer->frameInfo_.timePonit.startSmooth = GetNowTimeStampUs();
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        DHLOGD("DCameraStreamDataProcessProducer FeedStream devId %s dhId %s streamId: %d streamType: %d" +
            " streamSize: %d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
            streamId_, streamType_, buffer->Size());
        if (buffers_.size() >= DCAMERA_PRODUCER_MAX_BUFFER_SIZE) {
            DHLOGD("DCameraStreamDataProcessProducer FeedStream OverSize devId %s dhId %s streamType: %d" +
                " streamSize: %d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
                streamType_, buffer->Size());
            buffers_.pop_front();
        }
        if (streamType_ == SNAPSHOT_FRAME) {
            buffers_.push_back(buffer);
            producerCon_.notify_one();
        }
    }
    if (streamType_ == CONTINUOUS_FRAME) {
        smoother_->PushData(buffer);
    }
}

void DCameraStreamDataProcessProducer::StartEvent()
{
    prctl(PR_SET_NAME, SOURCE_START_EVENT.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

void DCameraStreamDataProcessProducer::LooperSnapShot()
{
    DHLOGI("LooperSnapShot producer devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
    std::string name = PRODUCER + std::to_string(streamType_);
    prctl(PR_SET_NAME, name.c_str());
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;

    while (state_ == DCAMERA_PRODUCER_STATE_START) {
        std::shared_ptr<DataBuffer> buffer = nullptr;
        {
            std::unique_lock<std::mutex> lock(bufferMutex_);
            producerCon_.wait(lock, [this] {
                return (!buffers_.empty() || state_ == DCAMERA_PRODUCER_STATE_STOP);
            });
            if (state_ == DCAMERA_PRODUCER_STATE_STOP) {
                continue;
            }

            if (!buffers_.empty()) {
                DHLOGI("LooperSnapShot producer get buffer devId: %s dhId: %s streamType: %d streamId: %d state: %d",
                    GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
                buffer = buffers_.front();
            }
        }

        if (buffer == nullptr) {
            DHLOGI("LooperSnapShot producer get buffer failed devId: %s dhId: %s streamType: %d streamId: %d state: %d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
            continue;
        }
        int32_t ret = FeedStreamToDriver(dhBase, buffer);
        if (ret != DCAMERA_OK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DCAMERA_PRODUCER_RETRY_SLEEP_MS));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(bufferMutex_);
            buffers_.pop_front();
        }
    }
    DHLOGI("LooperSnapShot producer end devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

int32_t DCameraStreamDataProcessProducer::FeedStreamToDriver(const DHBase& dhBase,
    const std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGD("LooperFeed devId %s dhId %s streamSize: %d streamType: %d, streamId: %d", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str(), buffer->Size(), streamType_, streamId_);
    if (camHdiProvider_ == nullptr) {
        DHLOGI("camHdiProvider is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    DCameraBuffer sharedMemory;
    int32_t ret = camHdiProvider_->AcquireBuffer(dhBase, streamId_, sharedMemory);
    if (ret != SUCCESS) {
        DHLOGE("AcquireBuffer devId: %s dhId: %s streamId: %d ret: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }

    do {
        ret = CheckSharedMemory(sharedMemory, buffer);
        if (ret != DCAMERA_OK) {
            DHLOGE("CheckSharedMemory failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str());
            break;
        }
        sharedMemory.bufferHandle_->GetBufferHandle()->virAddr =
            DCameraMemoryMap(sharedMemory.bufferHandle_->GetBufferHandle());
        if (sharedMemory.bufferHandle_->GetBufferHandle()->virAddr == nullptr) {
            DHLOGE("mmap failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
        }
        ret = memcpy_s(sharedMemory.bufferHandle_->GetBufferHandle()->virAddr, sharedMemory.size_, buffer->Data(),
            buffer->Size());
        if (ret != EOK) {
            DHLOGE("memcpy_s devId: %s dhId: %s streamId: %d bufSize: %d, addressSize: %d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, buffer->Size(),
                sharedMemory.size_);
            break;
        }
        sharedMemory.size_ = buffer->Size();
    } while (0);

    ret = camHdiProvider_->ShutterBuffer(dhBase, streamId_, sharedMemory);
    if (sharedMemory.bufferHandle_ != nullptr) {
        DCameraMemoryUnmap(sharedMemory.bufferHandle_->GetBufferHandle());
    }
    if (ret != SUCCESS) {
        DHLOGE("ShutterBuffer devId: %s dhId: %s streamId: %d ret: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }
    DHLOGD("LooperFeed end devId %s dhId %s streamSize: %d streamType: %d, streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), buffer->Size(), streamType_, streamId_);
    return ret;
}

int32_t DCameraStreamDataProcessProducer::CheckSharedMemory(const DCameraBuffer& sharedMemory,
    const std::shared_ptr<DataBuffer>& buffer)
{
    if (sharedMemory.bufferHandle_ == nullptr || sharedMemory.bufferHandle_->GetBufferHandle() == nullptr) {
        DHLOGE("bufferHandle read failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str());
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    if (buffer->Size() > sharedMemory.size_) {
        DHLOGE("sharedMemory devId: %s dhId: %s streamId: %d bufSize: %d, addressSize: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, buffer->Size(),
            sharedMemory.size_);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    return DCAMERA_OK;
}

void DCameraStreamDataProcessProducer::OnSmoothFinished(const std::shared_ptr<IFeedableData>& data)
{
    std::shared_ptr<DataBuffer> buffer = std::reinterpret_pointer_cast<DataBuffer>(data);
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;
    auto feedFunc = [this, dhBase, buffer]() {
        FeedStreamToDriver(dhBase, buffer);
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(feedFunc);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
