/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#include "AppImpl.h"
#include "AppMain.h"
#include "AppPlatformShellCommands.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/util/af.h>

#include "include/account-login/AccountLoginManager.h"
#include "include/application-basic/ApplicationBasicManager.h"
#include "include/application-launcher/ApplicationLauncherManager.h"
#include "include/audio-output/AudioOutputManager.h"
#include "include/channel/ChannelManager.h"
#include "include/content-launcher/ContentLauncherManager.h"
#include "include/keypad-input/KeypadInputManager.h"
#include "include/low-power/LowPowerManager.h"
#include "include/media-input/MediaInputManager.h"
#include "include/media-playback/MediaPlaybackManager.h"
#include "include/target-navigator/TargetNavigatorManager.h"
#include "include/wake-on-lan/WakeOnLanManager.h"

#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h>
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;
using namespace chip::Access;

bool emberAfBasicClusterMfgSpecificPingCallback(app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

namespace {
static AccountLoginManager accountLoginManager;
static ApplicationBasicManager applicationBasicManager;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static ApplicationLauncherManager applicationLauncherManager(true);
#else  // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static ApplicationLauncherManager applicationLauncherManager(false);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static AudioOutputManager audioOutputManager;
static ChannelManager channelManager;
static ContentLauncherManager contentLauncherManager;
static KeypadInputManager keypadInputManager;
static LowPowerManager lowPowerManager;
static MediaInputManager mediaInputManager;
static MediaPlaybackManager mediaPlaybackManager;
static TargetNavigatorManager targetNavigatorManager;
static WakeOnLanManager wakeOnLanManager;
} // namespace

void ApplicationInit() {}

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
class MyUserPrompter : public UserPrompter
{
    // tv should override this with a dialog prompt
    inline void PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptForCommissionPincode(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error) override { return; }
};

MyUserPrompter gMyUserPrompter;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
class MyPincodeService : public PincodeService
{
    uint32_t FetchCommissionPincodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId) override
    {
        return ContentAppPlatform::GetInstance().GetPincodeFromContentApp(vendorId, productId, rotatingId);
    }
};
MyPincodeService gMyPincodeService;

// extern CHIP_ERROR LogAccessControlEvent(const Access::AccessControl::Entry & entry,
//                                         const Access::SubjectDescriptor & subjectDescriptor,
//                                         Controller::AccessControlCluster::ChangeTypeEnum changeType);

class MyPostCommissioningListener : public PostCommissioningListener
{
    void CommissioningCompleted(uint16_t vendorId, uint16_t productId, NodeId nodeId, OperationalDeviceProxy * device) override
    {

        // TODO:
        // - the videoPlayerEndpointId chosen should come from the App Platform (determined based upon vid/pid of node)
        // - the cluster(s) chosen should come from the App Platform
        // - the contentAppEndpointId chosen should come from the App Platform (determined by which apps allow this vendorId)
        // - the number of contentAppEndpointId could be 0 or more
        // - the fabric index can't be 0, but needs to be validated
        constexpr EndpointId kBindingClusterEndpoint = 0;

        GroupId groupId                  = kUndefinedGroupId;
        EndpointId videoPlayerEndpointId = 1;
        EndpointId contentAppEndpointId  = 6;
        ClusterId clusterId              = kInvalidClusterId;
        FabricIndex fabricIndex          = 1;

        ChipLogProgress(Controller, "Attempting to create ACL");

        // size_t subjectIndex;
        // size_t targetIndex;
        Access::AccessControl::Entry::Target targets[] = {
            { .flags    = Access::AccessControl::Entry::Target::kEndpoint,
              .endpoint = contentAppEndpointId }, // grant access to the content app endpoint
            { .flags    = Access::AccessControl::Entry::Target::kCluster | Access::AccessControl::Entry::Target::kEndpoint,
              .cluster  = 0x050a,
              .endpoint = videoPlayerEndpointId }, // 0x050a is content launcher
        };

        Access::AccessControl::Entry entry;
        GetAccessControl().PrepareEntry(entry);
        entry.SetAuthMode(Access::AuthMode::kCase);
        entry.SetFabricIndex(fabricIndex);
        entry.SetPrivilege(Access::Privilege::kOperate);
        entry.AddSubject(nullptr, nodeId);
        for (auto & target : targets)
        {
            entry.AddTarget(nullptr, target);
        }

        Access::AuthMode a;
        FabricIndex f;
        Access::Privilege p;
        entry.GetAuthMode(a);
        entry.GetFabricIndex(f);
        entry.GetPrivilege(p);
        ChipLogProgress(Controller, "Entry authmode=%d fabricIndex=%d privilege=%d", static_cast<int>(a), f, static_cast<int>(p));

        GetAccessControl().CreateEntry(nullptr, entry, &fabricIndex);

        // SubjectDescriptor subjectDescriptor = { .fabricIndex = fabricIndex, .authMode = AuthMode::kCase, .subject = nodeId };
        // LogAccessControlEvent(entry, subjectDescriptor, Controller::AccessControlCluster::ChangeTypeEnum::kAdded);

        ChipLogProgress(Controller, "Attempting to create Binding");

        // todo: nodeId passed below should be this device's nodeId
        ContentAppPlatform::GetInstance().CreateBindingWithCallback(device, kBindingClusterEndpoint, nodeId, groupId,
                                                                    videoPlayerEndpointId, clusterId, OnSuccessResponse,
                                                                    OnFailureResponse);
    }

    /* Callback when command results in success */
    static void OnSuccessResponse(void * context)
    {
        ChipLogProgress(Controller, "OnSuccessResponse - Binding Add Successfully");
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->PostCommissioningSucceeded();
        }
    }

    /* Callback when command results in failure */
    static void OnFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnFailureResponse - Binding Add Failed");
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->PostCommissioningFailed(error);
        }
    }
};

MyPostCommissioningListener gMyPostCommissioningListener;

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

int main(int argc, char * argv[])
{

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentAppFactoryImpl factory;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentAppPlatform::GetInstance().SetupAppPlatform();
    ContentAppPlatform::GetInstance().SetContentAppFactory(&factory);
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
    if (cdc != nullptr)
    {
        cdc->SetPincodeService(&gMyPincodeService);
        cdc->SetUserPrompter(&gMyUserPrompter);
        cdc->SetPostCommissioningListener(&gMyPostCommissioningListener);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterAppPlatformCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}

void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ContentLauncher::SetDefaultDelegate");
    ContentLauncher::SetDefaultDelegate(endpoint, &contentLauncherManager);
}

void emberAfAccountLoginClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AccountLogin::SetDefaultDelegate");
    AccountLogin::SetDefaultDelegate(endpoint, &accountLoginManager);
}

void emberAfApplicationBasicClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationBasic::SetDefaultDelegate");
    ApplicationBasic::SetDefaultDelegate(endpoint, &applicationBasicManager);
}

void emberAfApplicationLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationLauncher::SetDefaultDelegate");
    ApplicationLauncher::SetDefaultDelegate(endpoint, &applicationLauncherManager);
}

void emberAfAudioOutputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AudioOutput::SetDefaultDelegate");
    AudioOutput::SetDefaultDelegate(endpoint, &audioOutputManager);
}

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Channel::SetDefaultDelegate");
    Channel::SetDefaultDelegate(endpoint, &channelManager);
}

void emberAfKeypadInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: KeypadInput::SetDefaultDelegate");
    KeypadInput::SetDefaultDelegate(endpoint, &keypadInputManager);
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}

void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaInput::SetDefaultDelegate");
    MediaInput::SetDefaultDelegate(endpoint, &mediaInputManager);
}

void emberAfMediaPlaybackClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaPlayback::SetDefaultDelegate");
    MediaPlayback::SetDefaultDelegate(endpoint, &mediaPlaybackManager);
}

void emberAfTargetNavigatorClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: TargetNavigator::SetDefaultDelegate");
    TargetNavigator::SetDefaultDelegate(endpoint, &targetNavigatorManager);
}

void emberAfWakeOnLanClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: WakeOnLanManager::SetDefaultDelegate");
    WakeOnLan::SetDefaultDelegate(endpoint, &wakeOnLanManager);
}
