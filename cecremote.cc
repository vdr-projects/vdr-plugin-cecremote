/*
 * cecremote.cc
 *
 *  Created on: 25.12.2014
 *      Author: uli
 */
#include "cecremote.h"
#include "ceclog.h"

// We need this for cecloader.h
#include <iostream>
using namespace std;
#include <cecloader.h>

cCECRemote::cCECRemote(int loglevel):
        cRemote("CEC"),
        cThread("CEC receiver"),
        mDevicesFound(0),
        mCECAdapter(NULL)
{
    mCECLogLevel = loglevel;
    Start();
    Dsyslog("cCECRemote start");
}


cCECRemote::~cCECRemote()
{
    Cancel(3);
    if (mCECAdapter != NULL) {
        mCECAdapter->SetInactiveView();
        mCECAdapter->Close();
        UnloadLibCec(mCECAdapter);
    }
}

void cCECRemote::Action(void)
{
    cCECCmd cmd;
    while (Running()) {
        cmd = WaitCmd();
        if (cmd.mCmd != 0) {
            Dsyslog ("Action %d Val %d Addr %d",
                     cmd.mCmd, cmd.mVal, cmd.mAddress);
        }
        switch (cmd.mCmd)
        {
        case CEC_KEYRPRESS:
            if (cmd.mVal >= 0 && cmd.mVal <= CEC_USER_CONTROL_CODE_MAX) {
                const cKeyList &inputKeys = mKeyMap[cmd.mVal];
                for (cKeyListIterator keys = inputKeys.begin();
                     keys != inputKeys.end(); ++keys) {
                    eKeys k = *keys;
                    Put(k);
                    Dsyslog ("   Put(%d)", k);
                }
            }
            break;
        case CEC_POWERON: // TODO
            Dsyslog("Power on");
            if (mCECAdapter->PowerOnDevices(cmd.mAddress) != 0) {
                Esyslog("PowerOnDevice failed for %s", mCECAdapter->ToString(cmd.mAddress));
            }
            break;
        case CEC_POWEROFF: // TODO
               Dsyslog("Power off");
               if (mCECAdapter->StandbyDevices(cmd.mAddress) != 0) {
                   Esyslog("PowerOnDevice failed for %s", mCECAdapter->ToString(cmd.mAddress));
               }
               break;
        case CEC_MAKEACTIVE:
            Dsyslog ("Make active");
            mCECAdapter->SetActiveSource();
            break;
        case CEC_MAKEINACTIVE:
            Dsyslog ("Make inactive");
            mCECAdapter->SetInactiveView();
            break;
        case CEC_VDRKEYPRESS:
            Dsyslog ("Send Keypress %d", cmd.mVal);
            break;
        case CEC_TIMEOUT:
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }

    }
}

int CecLogMessage(void *cbParam, const cec_log_message message)
{
    cCECRemote *rem = (cCECRemote *)cbParam;
    if ((message.level & rem->getCECLogLevel()) == message.level)
    {
        string strLevel;
        switch (message.level)
        {
        case CEC_LOG_ERROR:
            strLevel = "ERROR:   ";
            break;
        case CEC_LOG_WARNING:
            strLevel = "WARNING: ";
            break;
        case CEC_LOG_NOTICE:
            strLevel = "NOTICE:  ";
            break;
        case CEC_LOG_TRAFFIC:
            strLevel = "TRAFFIC: ";
            break;
        case CEC_LOG_DEBUG:
            strLevel = "DEBUG:   ";
            break;
        default:
            break;
        }

        char strFullLog[128]; // TODO
        snprintf(strFullLog, 127, "CEC %s %s", strLevel.c_str(), message.message);
        if (message.level == CEC_LOG_ERROR)
        {
            Esyslog(strFullLog);
        }
        else
        {
            Dsyslog(strFullLog);
        }
    }

    return 0;
}


int CecKeyPress(void *cbParam, const cec_keypress key)
{
    cCECRemote *rem = (cCECRemote *)cbParam;

    Dsyslog("key pressed %d (%d)", key.keycode, key.duration);
    if ((key.keycode >= 0) && (key.keycode <= CEC_USER_CONTROL_CODE_MAX) &&
        (key.duration == 0))
    {
        cCECCmd cmd(CEC_KEYRPRESS, (int)key.keycode);
        rem->PushCmd(cmd);
    }
    return 0;
}

int CecCommand(void *cbParam, const cec_command command)
{
    Dsyslog("CEC Command %d", command.opcode);
    return 0;
}

int CecAlert(void *cbParam, const libcec_alert type, const libcec_parameter param)
{
    Dsyslog("CecAlert %d", type);
    switch (type)
    {
    case CEC_ALERT_CONNECTION_LOST:
        /* TODO */
        Esyslog("Connection lost - trying to reconnect");

        break;
    default:
        break;
    }
    return 0;
}

bool cCECRemote::Initialize(void)
{
    // Initialize Callbacks
    mCECCallbacks.Clear();
    mCECCallbacks.CBCecLogMessage  = &::CecLogMessage;
    mCECCallbacks.CBCecKeyPress    = &::CecKeyPress;
    mCECCallbacks.CBCecCommand     = &::CecCommand;
    mCECCallbacks.CBCecAlert       = &::CecAlert;

    // Setup CEC configuration
    mCECConfig.Clear();
    strncpy(mCECConfig.strDeviceName, "VDR", sizeof(mCECConfig.strDeviceName));
    mCECConfig.clientVersion       = CEC_CLIENT_VERSION_CURRENT;
    mCECConfig.bActivateSource     = CEC_FALSE;
    mCECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
    mCECConfig.callbackParam = this;
    mCECConfig.callbacks = &mCECCallbacks;
    // Initialize libcec
    mCECAdapter = LibCecInitialise(&mCECConfig);
    if (mCECAdapter == NULL) {
        Esyslog("Can not initialize libcec");
        exit(-1);
    }
    // init video on targets that need this
    mCECAdapter->InitVideoStandalone();
    Dsyslog("LibCEC %s", mCECAdapter->GetLibInfo());

    mDevicesFound = mCECAdapter->DetectAdapters(mCECAdapterDescription,
                                                MAX_CEC_ADAPTERS, NULL);
    if (mDevicesFound <= 0)
    {
        Esyslog("No adapter found");
        UnloadLibCec(mCECAdapter);
        exit(-1);
    }
    for (int i = 0; i < mDevicesFound; i++)
    {
        Dsyslog("Device %d path: %s port: %s Firmware %04d", i,
                        mCECAdapterDescription[0].strComPath,
                        mCECAdapterDescription[0].strComName,
                        mCECAdapterDescription[0].iFirmwareVersion);
    }

    if (!mCECAdapter->Open(mCECAdapterDescription[0].strComName))
    {
      Esyslog("unable to open the device on port %s",
              mCECAdapterDescription[0].strComName);
      UnloadLibCec(mCECAdapter);
      exit(-1);
    }
    cec_logical_addresses devices = mCECAdapter->GetActiveDevices();
    for (int j = 0; j < 16; j++)
    {
        if (devices[j])
        {
            cec_logical_address logical_addres = (cec_logical_address) j;

            uint16_t phaddr = mCECAdapter->GetDevicePhysicalAddress(logical_addres);
            cec_osd_name name = mCECAdapter->GetDeviceOSDName(logical_addres);
            cec_vendor_id vendor = (cec_vendor_id)mCECAdapter->GetDeviceVendorId(logical_addres);
            Dsyslog("  %s@%04x %15.15s %15.15s",
                    mCECAdapter->ToString(logical_addres),
                    phaddr, name.name,
                    mCECAdapter->ToString(vendor));
        }
    }

    mKeyMap.resize(CEC_USER_CONTROL_CODE_MAX + 1, {});
    mKeyMap[CEC_USER_CONTROL_CODE_SELECT                      ] = { kOk };
    mKeyMap[CEC_USER_CONTROL_CODE_UP                          ] = { kUp };
    mKeyMap[CEC_USER_CONTROL_CODE_DOWN                        ] = { kDown };
    mKeyMap[CEC_USER_CONTROL_CODE_LEFT                        ] = { kLeft };
    mKeyMap[CEC_USER_CONTROL_CODE_RIGHT                       ] = { kRight };
    mKeyMap[CEC_USER_CONTROL_CODE_RIGHT_UP                    ] = { kRight, kUp };
    mKeyMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN                  ] = { kRight, kDown };
    mKeyMap[CEC_USER_CONTROL_CODE_LEFT_UP                     ] = { kLeft, kUp };
    mKeyMap[CEC_USER_CONTROL_CODE_LEFT_DOWN                   ] = { kRight, kUp };
    mKeyMap[CEC_USER_CONTROL_CODE_ROOT_MENU                   ] = { kMenu };
    mKeyMap[CEC_USER_CONTROL_CODE_SETUP_MENU                  ] = { kSetup };
    mKeyMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU               ] = { kSetup };
    mKeyMap[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_EXIT                        ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER0                     ] = { k0 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER1                     ] = { k1 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER2                     ] = { k2 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER3                     ] = { k3 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER4                     ] = { k4 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER5                     ] = { k5 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER6                     ] = { k6 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER7                     ] = { k7 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER8                     ] = { k8 };
    mKeyMap[CEC_USER_CONTROL_CODE_NUMBER9                     ] = { k9 };
    mKeyMap[CEC_USER_CONTROL_CODE_DOT                         ] = {  };
    mKeyMap[CEC_USER_CONTROL_CODE_ENTER                       ] = { kOk };
    mKeyMap[CEC_USER_CONTROL_CODE_CLEAR                       ] = { kBack };
    mKeyMap[CEC_USER_CONTROL_CODE_NEXT_FAVORITE               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_UP                  ] = { kChanUp };
    mKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN                ] = { kChanDn };
    mKeyMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL            ] = { kChanPrev };
    mKeyMap[CEC_USER_CONTROL_CODE_SOUND_SELECT                ] = { kAudio };
    mKeyMap[CEC_USER_CONTROL_CODE_INPUT_SELECT                ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION         ] = { kInfo };
    mKeyMap[CEC_USER_CONTROL_CODE_HELP                        ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_PAGE_UP                     ] = { kNext };
    mKeyMap[CEC_USER_CONTROL_CODE_PAGE_DOWN                   ] = { kPrev };
    mKeyMap[CEC_USER_CONTROL_CODE_POWER                       ] = { kPower };
    mKeyMap[CEC_USER_CONTROL_CODE_VOLUME_UP                   ] = { kVolUp };
    mKeyMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN                 ] = { kVolDn };
    mKeyMap[CEC_USER_CONTROL_CODE_MUTE                        ] = { kMute };
    mKeyMap[CEC_USER_CONTROL_CODE_PLAY                        ] = { kPlay };
    mKeyMap[CEC_USER_CONTROL_CODE_STOP                        ] = { kStop };
    mKeyMap[CEC_USER_CONTROL_CODE_PAUSE                       ] = { kPause };
    mKeyMap[CEC_USER_CONTROL_CODE_RECORD                      ] = { kRecord };
    mKeyMap[CEC_USER_CONTROL_CODE_REWIND                      ] = { kFastRew };
    mKeyMap[CEC_USER_CONTROL_CODE_FAST_FORWARD                ] = { kFastFwd };
    mKeyMap[CEC_USER_CONTROL_CODE_EJECT                       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_FORWARD                     ] = { kFastFwd };
    mKeyMap[CEC_USER_CONTROL_CODE_BACKWARD                    ] = { kFastRew };
    mKeyMap[CEC_USER_CONTROL_CODE_STOP_RECORD                 ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD                ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_ANGLE                       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_SUB_PICTURE                 ] = { kSubtitles };
    mKeyMap[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_PLAY_FUNCTION               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_RECORD_FUNCTION             ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_STOP_FUNCTION               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_MUTE_FUNCTION               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_TUNE_FUNCTION               ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_F1_BLUE                     ] = { kBlue };
    mKeyMap[CEC_USER_CONTROL_CODE_F2_RED                      ] = { kRed };
    mKeyMap[CEC_USER_CONTROL_CODE_F3_GREEN                    ] = { kGreen };
    mKeyMap[CEC_USER_CONTROL_CODE_F4_YELLOW                   ] = { kYellow };
    mKeyMap[CEC_USER_CONTROL_CODE_F5                          ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_DATA                        ] = { };
    mKeyMap[CEC_USER_CONTROL_CODE_AN_RETURN                   ] = { kBack };
    mKeyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST            ] = { };

    return false;
}

void cCECRemote::PushCmd(const cCECCmd &cmd)
{
    cMutexLock lock(&mQueueMutex);
    mQueue.push(cmd);
    mQueueWait.Signal();
}

cCECCmd cCECRemote::WaitCmd()
{
    cCECCmd cmd;
    mQueueMutex.Lock();
    if (mQueue.empty()) {
        mQueueMutex.Unlock();
        mQueueWait.Wait(1000);
        mQueueMutex.Lock();
    }
    if (mQueue.empty()) {
        cmd.mCmd = CEC_TIMEOUT;
    }
    else {
        cmd = mQueue.front();
        mQueue.pop();
    }
    mQueueMutex.Unlock();
    return cmd;
}
