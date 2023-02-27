/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "dcamera_handler.h"
#define private public
#include "distributed_camera_sink_service.h"
#undef private
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_camera_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSinkServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DistributedCameraSinkService> sinkService_;
};

std::string g_dhId;
std::string g_testParams = "TestParams";
std::string g_testCameraInfo = "";
std::string g_testChannelInfoContinue = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId",
    "Detail": [{"DataSessionFlag": "dataContinue", "StreamType": 0}]}
})";
std::string g_testOpenInfoService = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

void DistributedCameraSinkServiceTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DistributedCameraSinkServiceTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DistributedCameraSinkServiceTest::SetUp(void)
{
    DHLOGI("enter");
    sinkService_ = std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    DCameraHandler::GetInstance().Initialize();
    g_dhId = DCameraHandler::GetInstance().GetCameras().front();
}

void DistributedCameraSinkServiceTest::TearDown(void)
{
    DHLOGI("enter");
    sinkService_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_service_test_001
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_001");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->SubscribeLocalHardware(g_dhId, g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_002
 * @tc.desc: Verify the UnSubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_002");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->UnsubscribeLocalHardware(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_003
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_003");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->StopCapture(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_004
 * @tc.desc: Verify the ChannelNeg function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_004");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ChannelNeg(g_dhId, g_testChannelInfoContinue);
    EXPECT_NE(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_005
 * @tc.desc: Verify the GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_005");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->GetCameraInfo(g_dhId, g_testCameraInfo);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_006
 * @tc.desc: Verify the OpenChannel and CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_006");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->InitSink(g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->OpenChannel(g_dhId, g_testOpenInfoService);
    EXPECT_NE(DCAMERA_OK, ret);

    ret = sinkService_->ChannelNeg(g_dhId, g_testChannelInfoContinue);
    EXPECT_NE(DCAMERA_OK, ret);

    ret = sinkService_->CloseChannel(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = sinkService_->ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_007
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_007");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t fd = 0;
    std::vector<std::u16string> args;
    std::u16string str(u"");
    args.push_back(str);
    int ret = sinkService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS