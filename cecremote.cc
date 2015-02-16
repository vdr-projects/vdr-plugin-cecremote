/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the remote receiving and processing the CEC commands.
 */

#include "cecremote.h"
#include "ceclog.h"

// We need this for cecloader.h
#include <iostream>
using namespace std;
#include <cecloader.h>

eKeys cCECRemote::mDefaultKeyMap[CEC_USER_CONTROL_CODE_MAX+2][2];
// Callback for CEC KeyPress
static int CecKeyPressCallback(void *cbParam, const cec_keypress key)
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
static int CecCommandCallback(void *cbParam, const cec_command command)
{
    cCECRemote *rem = (cCECRemote *)cbParam;
    Dsyslog("CEC Command %d : %s", command.opcode, rem->mCECAdapter->ToString(command.opcode));
    return 0;
}


// Callback for CEC Alert
static int CecAlertCallback(void *cbParam, const libcec_alert type, const libcec_parameter param)
{
    Dsyslog("CecAlert %d)", type);
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

static int CecLogMessageCallback(void *cbParam, const cec_log_message message)
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

static void CECSourceActivatedCallback (void *cbParam,
                                        const cec_logical_address address,
                                        const uint8_t activated)
{
    Dsyslog("CECSourceActivatedCallback adress %d activated %d", address, activated);
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

    Dsyslog("cCECRemote start worker thread");
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
            if (mCECAdapter->PowerOnDevices(cmd.mAddress) != 0) {
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
            mCECAdapter->SetActiveSource(); /* TODO check */
            break;
        case CEC_MAKEINACTIVE:
            Dsyslog ("Make inactive");
            mCECAdapter->SetInactiveView(); /* TODO check */
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
        case CEC_EXECSHELL:
            Dsyslog ("Exec: %s", cmd.mExec.c_str());
            if (system(cmd.mExec.c_str()) < 0) {
                Esyslog("Exec failed");
            }
            break;
        case CEC_TIMEOUT:
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }
    }
    Dsyslog("cCECRemote stop worker thread");
}

void cCECRemote::InitKeyFromDefault(cVdrKeyMap &map)
{
    map.resize(CEC_USER_CONTROL_CODE_MAX + 2);
    for (int i = 0; i < CEC_USER_CONTROL_CODE_MAX+1; i++) {
        map[i].clear();
        if (mDefaultKeyMap[i][0] != kNone) {
            map[i].push_back(mDefaultKeyMap[i][0]);
        }
        if (mDefaultKeyMap[i][1] != kNone) {
            map[i].push_back(mDefaultKeyMap[i][1]);
        }
    }
    // Empty list
    map[CEC_USER_CONTROL_CODE_MAX+1].clear();
}

bool cCECRemote::Initialize(void)
{
    Dsyslog("cCECRemote::Initialize");
    // Initialize Callbacks
    mCECCallbacks.Clear();
    mCECCallbacks.CBCecLogMessage  = &::CecLogMessageCallback;
    mCECCallbacks.CBCecKeyPress    = &::CecKeyPressCallback;
    mCECCallbacks.CBCecCommand     = &::CecCommandCallback;
    mCECCallbacks.CBCecAlert       = &::CecAlertCallback;
    mCECCallbacks.CBCecSourceActivated = &::CECSourceActivatedCallback;

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

    for (int i = 0; i < CEC_USER_CONTROL_CODE_MAX+1; i++) {
        mDefaultKeyMap[i][0] = kNone;
        mDefaultKeyMap[i][1] = kNone;
    }

    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SELECT             ][0] = kOk;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_UP                 ][0] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DOWN               ][0] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT               ][0] = kLeft;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT              ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_UP           ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_UP           ][1] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN         ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN         ][1] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_UP            ][0] = kLeft;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_UP            ][1] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_DOWN          ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_DOWN          ][1] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ROOT_MENU          ][0] = kMenu;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SETUP_MENU         ][0] = kSetup;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU      ][0] = kSetup;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER0            ][0] = k0;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER1            ][0] = k1;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER2            ][0] = k2;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER3            ][0] = k3;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER4            ][0] = k4;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER5            ][0] = k5;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER6            ][0] = k6;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER7            ][0] = k7;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER8            ][0] = k8;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER9            ][0] = k9;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ENTER              ][0] = kOk;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CLEAR              ][0] = kBack;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_UP         ][0] = kChanUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN       ][0] = kChanDn;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL   ][0] = kChanPrev;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SOUND_SELECT       ][0] = kAudio;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION][0] = kInfo;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAGE_UP            ][0] = kNext;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAGE_DOWN          ][0] = kPrev;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_POWER              ][0] = kPower;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_VOLUME_UP          ][0] = kVolUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN        ][0] = kVolDn;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_MUTE               ][0] = kMute;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PLAY               ][0] = kPlay;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_STOP               ][0] = kStop;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAUSE              ][0] = kPause;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RECORD             ][0] = kRecord;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_REWIND             ][0] = kFastRew;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_FAST_FORWARD       ][0] = kFastFwd;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_FORWARD            ][0] = kFastFwd;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_BACKWARD           ][0] = kFastRew;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SUB_PICTURE        ][0] = kSubtitles;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F1_BLUE            ][0] = kBlue;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F2_RED             ][0] = kRed;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F3_GREEN           ][0] = kGreen;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F4_YELLOW          ][0] = kYellow;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_AN_RETURN          ][0] = kBack;

/*   mDefaultKeyMap[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PLAY_FUNCTION               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RECORD_FUNCTION             ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_STOP_FUNCTION               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_MUTE_FUNCTION               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_TUNE_FUNCTION               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           ] = { }
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F5                          ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DATA                        ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_STOP_RECORD                 ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD                ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ANGLE                       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_EJECT                       ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_EXIT                        ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DOT                         ] = {  };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NEXT_FAVORITE               ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_INPUT_SELECT                ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_HELP                        ] = { };
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST            ] = { };*/

    Dsyslog("Load keymap");
    InitKeyFromDefault(mKeyMap);

    Dsyslog("END cCECRemote::Initialize");
    return false;
}

void cCECRemote::ExecToggle(cec_logical_address addr,
                            const cCmdQueue &poweron, const cCmdQueue &poweroff)
{
    // Wait until queue is empty
    cCondWait w;
    bool full = true;
    while (full) {
        mQueueMutex.Lock();
        full = !mQueue.empty();
        mQueueMutex.Unlock();
        if (full) {
            w.Wait(100);
       }
    }

    cec_power_status status = mCECAdapter->GetDevicePowerStatus(addr);
    Dsyslog("ExecToggle: %s", mCECAdapter->ToString(status));

    if (status == CEC_POWER_STATUS_ON) {
        ExecCmd(poweroff);
    }
    else {
        ExecCmd(poweron);
    }
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
    Dsyslog("cCECRemote::PushCmd %d (size %d)", cmd.mCmd, mQueue.size());
    cMutexLock lock(&mQueueMutex);
    mQueueMutex.Lock();
    mQueue.push_back(cmd);
    mQueueMutex.Unlock();
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
