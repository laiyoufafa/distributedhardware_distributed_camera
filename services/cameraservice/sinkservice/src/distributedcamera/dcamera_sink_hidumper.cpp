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

#include "dcamera_sink_hidumper.h"

#include <iterator>

#include "distributed_camera_errno.h"
#include "distributed_camera_sink_service.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DcameraSinkHidumper);

namespace {
const std::string ARGS_HELP = "-h";
const std::string ARGS_VERSION_INFO = "--version";
const std::string ARGS_CAMERA_INFO = "--camNum";
const std::string ARGS_OPENED_INFO = "--opened";

const std::map<std::string, HidumpFlag> ARGS_MAP = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { ARGS_CAMERA_INFO, HidumpFlag::GET_CAMERA_INFO },
    { ARGS_OPENED_INFO, HidumpFlag::GET_OPENED_INFO },
    { ARGS_VERSION_INFO, HidumpFlag::GET_VERSION_INFO },
};
}

void DcameraSinkHidumper::SetSinkDumpInfo(CameraDumpInfo& camDumpInfo_)
{
    DistributedCameraSinkService::GetCamDumpInfo(camDumpInfo_);
}

bool DcameraSinkHidumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    DHLOGI("DcameraSinkHidumper Dump args.size():%d.", args.size());
    result.clear();
    int32_t argsSize = static_cast<int32_t>(args.size());
    for (int i = 0; i < argsSize; i++) {
        DHLOGI("DcameraSinkHidumper Dump args[%d]: %s.", i, args.at(i).c_str());
    }

    int32_t ret = ProcessDump(args[0], result);
    return ret;
}

int32_t DcameraSinkHidumper::ProcessDump(const std::string& args, std::string& result)
{
    DHLOGI("ProcessDump Dump.");
    HidumpFlag hf = HidumpFlag::UNKNOW;
    auto operatorIter = ARGS_MAP.find(args);
    if (operatorIter != ARGS_MAP.end()) {
        hf = operatorIter->second;
    }

    if (hf == HidumpFlag::GET_HELP) {
        ShowHelp(result);
        return DCAMERA_OK;
    }
    result.clear();
    SetSinkDumpInfo(camDumpInfo_);
    int32_t ret = DCAMERA_BAD_VALUE;
    switch (hf) {
        case HidumpFlag::GET_CAMERA_INFO: {
            ret = GetLocalCameraNumber(result);
            break;
        }
        case HidumpFlag::GET_OPENED_INFO: {
            ret = GetOpenedCameraInfo(result);
            break;
        }
        case HidumpFlag::GET_VERSION_INFO: {
            ret = GetVersionInfo(result);
            break;
        }
        default: {
            ret = ShowIllegalInfomation(result);
            break;
        }
    }

    return ret;
}

int32_t DcameraSinkHidumper::GetLocalCameraNumber(std::string& result)
{
    DHLOGI("GetLocalCameraNumber Dump.");
    result.append("CameraNumber\n")
          .append(std::to_string(camDumpInfo_.camNumber));
    return DCAMERA_OK;
}

int32_t DcameraSinkHidumper::GetOpenedCameraInfo(std::string& result)
{
    DHLOGI("GetOpenedCameraInfo Dump.");
    result.append("OpenedCamera\n")
          .append(camDumpInfo_.dhOpened);
    return DCAMERA_OK;
}

int32_t DcameraSinkHidumper::GetVersionInfo(std::string& result)
{
    DHLOGI("GetVersionInfo Dump.");
    result.append("CameraVersion\n")
          .append(camDumpInfo_.version);
    return DCAMERA_OK;
}

void DcameraSinkHidumper::ShowHelp(std::string& result)
{
    DHLOGI("ShowHelp Dump.");
    result.append("Usage:dump  <command> [options]\n")
          .append("Description:\n")
          .append("--version         ")
          .append("dump camera version in the system\n")
          .append("--camNum      ")
          .append("dump local camera numbers in the system\n")
          .append("--opened    ")
          .append("dump the opened camera in the system\n");
}

int32_t DcameraSinkHidumper::ShowIllegalInfomation(std::string& result)
{
    DHLOGI("ShowIllegalInfomation Dump.");
    result.append("unkown command");
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS