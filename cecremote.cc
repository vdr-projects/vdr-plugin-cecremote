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

// Callback for CEC KeyPress
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

// Callback for CEC Command
int CecCommand(void *cbParam, const cec_command command)
{
    cCECRemote *rem = (cCECRemote *)cbParam;
    Dsyslog("CEC Command %d : %s", command.opcode, rem->mCECAdapter->ToString(command.opcode));
    return 0;
}


// Callback for CEC Alert
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

/*
 * CEC remote
 */

cCECRemote::cCECRemote(int loglevel, const cCmdQueue &onStart,
                       const cCmdQueue &onStop):
        cRemote("CEC"),
        cThread("CEC receiver"),
        mDevicesFound(0)
{
    mCECAdapter = NULL;
    mCECLogLevel = loglevel;
    mOnStart = onStart;
    mOnStop = onStop;
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

cKeyList &cCECRemote::CECtoVDRKey(cec_user_control_code code)
{
    if ((code >= 0) && (code <= CEC_USER_CONTROL_CODE_MAX)) {
        return mKeyMap[code];
    }
    return mKeyMap[CEC_USER_CONTROL_CODE_MAX+1]; // Empty list
}

cec_user_control_code cCECRemote::VDRtoCECKey(eKeys key)
{
    int i;
    cKeyList inputKeys;

    for (i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++)
    {
        inputKeys = CECtoVDRKey((cec_user_control_code)i);
        for (cKeyListIterator keys = inputKeys.begin();
             keys != inputKeys.end(); ++keys) {
            eKeys k = *keys;
            if (k == key) {
                return (cec_user_control_code)i;
            }
        }
    }
    return CEC_USER_CONTROL_CODE_UNKNOWN;
}

bool cCECRemote::TextViewOn(cec_logical_address address)
{
    cec_command data;

    cec_command::Format(data, mCECConfig.baseDevice, address, CEC_OPCODE_TEXT_VIEW_ON);
    Dsyslog("Text View on : %02x %02x %02x", data.initiator, data.destination, data.opcode);
    return mCECAdapter->Transmit(data);
}

void cCECRemote::Action(void)
{
    cCECCmd cmd;
    cec_user_control_code ceckey;
    eKeys k;

    while (Running()) {
        cmd = WaitCmd();
        if (cmd.mCmd != CEC_TIMEOUT) {
            Dsyslog ("Action %d Val %d Addr %d",
                    cmd.mCmd, cmd.mVal, cmd.mAddress);
        }
        switch (cmd.mCmd)
        {
        case CEC_KEYRPRESS:
            if ((cmd.mVal >= 0) && (cmd.mVal <= CEC_USER_CONTROL_CODE_MAX)) {
                const cKeyList &inputKeys = mKeyMap[cmd.mVal];
                cKeyListIterator ikeys;
                for (ikeys = inputKeys.begin(); ikeys != inputKeys.end(); ++ikeys) {
                    k = *ikeys;
                    Put(k);
                    Dsyslog ("   Put(%d)", k);
                }
            }
            break;
        case CEC_POWERON: // TODO
            Dsyslog("Power on");
            if (TextViewOn(cmd.mAddress) != 0) {
                Esyslog("PowerOnDevice failed for %s",
                        mCECAdapter->ToString(cmd.mAddress));
            }
            break;
        case CEC_POWEROFF: // TODO
            Dsyslog("Power off");
            if (mCECAdapter->StandbyDevices(cmd.mAddress) != 0) {
                Esyslog("PowerOnDevice failed for %s",
                        mCECAdapter->ToString(cmd.mAddress));
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
            ceckey = VDRtoCECKey((eKeys)cmd.mVal);
            Dsyslog ("Send Keypress VDR %d - > CEC 0x%02x", cmd.mVal, ceckey);
            if (ceckey != CEC_USER_CONTROL_CODE_UNKNOWN) {
                if (!mCECAdapter->SendKeypress(cmd.mAddress, ceckey, true)) {
                    Esyslog("Keypress to %d %s failed",
                            cmd.mAddress, mCECAdapter->ToString(cmd.mAddress));
                    return;
                }
                cCondWait::SleepMs(50);
                if (!mCECAdapter->SendKeyRelease(cmd.mAddress, true)) {
                    Esyslog("SendKeyRelease to %d %s failed",
                            cmd.mAddress, mCECAdapter->ToString(cmd.mAddress));
                }
            }
            break;
        case CEC_TIMEOUT:
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }
    }
}

bool cCECRemote::Initialize(void)
{
    Dsyslog("cCECRemote::Initialize");
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
    mCECConfig.bActivateSource     = CEC_TRUE;
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

    Dsyslog("Load keymap");
    mKeyMap.resize(CEC_USER_CONTROL_CODE_MAX + 2, {});
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
    // Empty list
    mKeyMap[CEC_USER_CONTROL_CODE_MAX+1                       ] = { };

    Dsyslog("END cCECRemote::Initialize");
    return false;
}

void cCECRemote::ExecCmd(const cCmdQueue &cmdList)
{
    for (cCmdQueueIterator i = cmdList.begin();
           i != cmdList.end(); i++) {
        PushCmd(*i);
    }
}

void cCECRemote::PushCmd(const cCECCmd &cmd)
{
    Dsyslog("cCECRemote::PushCmd %d", cmd.mCmd);
    cMutexLock lock(&mQueueMutex);
    mQueue.push_back(cmd);
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
        mQueue.pop_front();
    }
    mQueueMutex.Unlock();
    return cmd;
}
