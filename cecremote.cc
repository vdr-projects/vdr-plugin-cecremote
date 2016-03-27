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
#include "cecremoteplugin.h"
#include <sys/wait.h>
// We need this for cecloader.h
#include <iostream>
using namespace std;
#include <cecloader.h>

const char *cCECRemote::VDRNAME = "VDR";
/*
 * Callback when libCEC receives a key press
 */
static int CecKeyPressCallback(void *cbParam, const cec_keypress key)
{
    static cec_user_control_code lastkey = CEC_USER_CONTROL_CODE_UNKNOWN;
    cCECRemote *rem = (cCECRemote *)cbParam;

    Dsyslog("key pressed %02x (%d)", key.keycode, key.duration);
    if (
        ((key.keycode >= 0) && (key.keycode <= CEC_USER_CONTROL_CODE_MAX)) &&
        ((key.duration == 0) || (key.keycode != lastkey))
       )
    {
        lastkey = key.keycode;
        cCECCmd cmd(CEC_KEYRPRESS, (int)key.keycode);
        rem->PushCmd(cmd);
    }
    return 0;
}

/*
 * Callback function for libCEC command messages.
 * Currently only used for debugging.
 */
static int CecCommandCallback(void *cbParam, const cec_command command)
{
    cCECRemote *rem = (cCECRemote *)cbParam;
    Dsyslog("CEC Command %d : %s Init %d Dest %d", command.opcode,
                                   rem->mCECAdapter->ToString(command.opcode),
                                   command.initiator, command.destination);
    switch (command.opcode)
    {
    case CEC_OPCODE_ACTIVE_SOURCE:
        {
            cCECCmd cmd(CEC_ACTIVE_SOURCE, (int)command.initiator);
            rem->PushCmd(cmd);
        }
        break;
    default:
        break;
    }
    return 0;
}


/*
 * Callback function for libCEC alert messages.
 * Currently only used for debugging.
 */
static int CecAlertCallback(void *cbParam, const libcec_alert type,
                            const libcec_parameter param)
{
    cCECRemote *rem = (cCECRemote *)cbParam;
    Dsyslog("CecAlert %d", type);
    switch (type)
    {
    case CEC_ALERT_CONNECTION_LOST:
        Esyslog("Connection lost");
        rem->Reconnect();
        break;
    case CEC_ALERT_TV_POLL_FAILED:
        Isyslog("TV Poll failed");
        break;
    case CEC_ALERT_SERVICE_DEVICE:
        Isyslog("CEC_ALERT_SERVICE_DEVICE");
        break;
    case CEC_ALERT_PERMISSION_ERROR:
        Isyslog("CEC_ALERT_PERMISSION_ERROR");
        break;
    case CEC_ALERT_PORT_BUSY:
        Isyslog("CEC_ALERT_PORT_BUSY");
        break;
    case CEC_ALERT_PHYSICAL_ADDRESS_ERROR:
        Isyslog("CEC_ALERT_PHYSICAL_ADDRESS_ERROR");
        break;
    default:
        break;
    }
    return 0;
}

/*
 * Callback function for libCEC to print out log messages.
 */
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

        char strFullLog[1040];
        snprintf(strFullLog, 1039, "CEC %s %s", strLevel.c_str(), message.message);
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
 * Callback function for libCEC when a device gets activated.
 * Currently only used for debugging.
 */
static void CECSourceActivatedCallback (void *cbParam,
                                        const cec_logical_address address,
                                        const uint8_t activated)
{
    Dsyslog("CECSourceActivatedCallback adress %d activated %d", address, activated);
}

/*
 * Callback function for libCEC when configuration changes.
 * Currently only used for debugging.
 */
static int CECConfigurationCallback (void *cbParam,
                                     const libcec_configuration config)
{
    Dsyslog("CECConfiguration");
    return CEC_TRUE;
}

/*
 * CEC remote constructor.
 * Initializes libCEC and the connected CEC adaptor.
 */

cCECRemote::cCECRemote(const cCECGlobalOptions &options, cPluginCecremote *plugin):
        cRemote("CEC"),
        cThread("CEC receiver"),
        mProcessedSerial(-1),
        mDevicesFound(0),
        mInExec(false)
{
    mPlugin = plugin;
    mCECAdapter = NULL;
    mHDMIPort = options.mHDMIPort;
    mBaseDevice = options.mBaseDevice;
    mCECLogLevel = options.cec_debug;
    mOnStart = options.mOnStart;
    mOnStop = options.mOnStop;
    mOnManualStart = options.mOnManualStart;
    mComboKeyTimeoutMs = options.mComboKeyTimeoutMs;
    mDeviceTypes = options.mDeviceTypes;
    mShutdownOnStandby = options.mShutdownOnStandby;
    mPowerOffOnStandby = options.mPowerOffOnStandby;

    SetDescription("CEC Action Thread");
    Connect();
    Start();
    if (mCECAdapter == NULL) {
        Esyslog("Can not initialize/connect to libcec");
        return;
    }

    Dsyslog("cCECRemote Init");
}

void cCECRemote::Startup()
{
    Dsyslog("cCECRemote Startup");

    if (mPlugin->GetStartManually()) {
        PushCmdQueue(mOnManualStart);
    }
    PushCmdQueue(mOnStart);
}

void cCECRemote::Connect()
{
    Dsyslog("cCECRemote::Connect");
    if (mCECAdapter != NULL) {
        return;
    }
    // Initialize Callbacks
    mCECCallbacks.Clear();
    mCECCallbacks.CBCecLogMessage  = &::CecLogMessageCallback;
    mCECCallbacks.CBCecKeyPress    = &::CecKeyPressCallback;
    mCECCallbacks.CBCecCommand     = &::CecCommandCallback;
    mCECCallbacks.CBCecAlert       = &::CecAlertCallback;
    mCECCallbacks.CBCecSourceActivated = &::CECSourceActivatedCallback;
    mCECCallbacks.CBCecConfigurationChanged = &::CECConfigurationCallback;

    // Setup CEC configuration
    mCECConfig.Clear();
    strncpy(mCECConfig.strDeviceName, VDRNAME, sizeof(mCECConfig.strDeviceName));

    // LibCEC before 3.0.0
#ifdef CEC_CLIENT_VERSION_CURRENT
    mCECConfig.clientVersion      = CEC_CLIENT_VERSION_CURRENT;
#else
    // LibCEC 3.0.0
    mCECConfig.clientVersion      = LIBCEC_VERSION_CURRENT;
#endif
    mCECConfig.bActivateSource    = CEC_FALSE;
    mCECConfig.iComboKeyTimeoutMs = mComboKeyTimeoutMs;
    mCECConfig.iHDMIPort = mHDMIPort;
    mCECConfig.wakeDevices.Clear();
    mCECConfig.powerOffDevices.Clear();
    mCECConfig.bShutdownOnStandby = mShutdownOnStandby;
    mCECConfig.bPowerOffOnStandby = mPowerOffOnStandby;
    mCECConfig.baseDevice = mBaseDevice;
    // If no <cecdevicetype> is specified in the <global>, set default
    if (mDeviceTypes.empty())
    {
        mCECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
    }
    else {
        // Add all device types as specified in <cecdevicetype>
        deviceTypeListIterator idev;
        for (idev = mDeviceTypes.begin();
                idev != mDeviceTypes.end(); ++idev) {
            cec_device_type t = *idev;
            mCECConfig.deviceTypes.Add(t);
            Dsyslog ("   Add device %d", t);
        }
    }

    // Setup callbacks
    mCECConfig.callbackParam = this;
    mCECConfig.callbacks = &mCECCallbacks;
    // Initialize libcec
    mCECAdapter = LibCecInitialise(&mCECConfig);
    if (mCECAdapter == NULL) {
        Esyslog("Can not initialize libcec");
        return;
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
        mCECAdapter = NULL;
        return;
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
        mCECAdapter = NULL;
        return;
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
            Dsyslog("   %15.15s %d@%04x %15.15s %15.15s",
                    mCECAdapter->ToString(logical_addres),
                    logical_addres, phaddr, name.name,
                    mCECAdapter->ToString(vendor));
        }
    }
    Dsyslog("END cCECRemote::Initialize");
}

void cCECRemote::Disconnect()
{
    if (mCECAdapter != NULL) {
        mCECAdapter->SetInactiveView();
        mCECAdapter->Close();
        UnloadLibCec(mCECAdapter);
    }
    mCECAdapter = NULL;
    Dsyslog("cCECRemote::Disconnect");
}

void cCECRemote::Stop()
{
    Dsyslog("Executing onStop");
    PushCmdQueue(mOnStop);
    // Send exit command to worker thread
    cCECCmd cmd(CEC_EXIT);
    PushWaitCmd(cmd);
    Dsyslog("onStop OK");
}
/*
 * Destructor stops the worker thread and unloads libCEC.
 */
cCECRemote::~cCECRemote()
{
    Cancel(3);
    Disconnect();
}

/*
 * Function to list all active CEC devices.
 */
cString cCECRemote::ListDevices()
{
    cString s = "Available CEC Devices:";
    uint16_t phaddr;
    cec_osd_name name;
    cec_vendor_id vendor;
    cec_power_status powerstatus;

    if (mCECAdapter == NULL) {
        Esyslog ("ListDevices CEC Adapter disconnected");
        s = "CEC Adapter disconnected";
        return s;
    }

    for (int i = 0; i < mDevicesFound; i++)
    {
        s = cString::sprintf("%s\n  Device %d path: %s port: %s Firmware %04d",
                             *s, i,
                             mCECAdapterDescription[0].strComPath,
                             mCECAdapterDescription[0].strComName,
                             mCECAdapterDescription[0].iFirmwareVersion);
    }

    s = cString::sprintf("%s\n\nActive Devices:", *s);
    cec_logical_addresses devices = mCECAdapter->GetActiveDevices();
    cec_logical_addresses own = mCECAdapter->GetLogicalAddresses();

    for (int j = 0; j < 16; j++)
    {
        if (devices[j])
        {
            cec_logical_address logical_addres = (cec_logical_address)j;

            phaddr = mCECAdapter->GetDevicePhysicalAddress(logical_addres);
            name = mCECAdapter->GetDeviceOSDName(logical_addres);
            vendor = (cec_vendor_id)mCECAdapter->GetDeviceVendorId(logical_addres);

            if (own[j]) {
                s = cString::sprintf("%s\n   %d# %-15.15s@%04x %-15.15s %-14.14s %-15.15s", *s,
                        logical_addres,
                        mCECAdapter->ToString(logical_addres),
                        phaddr, name.name,
                        VDRNAME, VDRNAME);
            }
            else {
                powerstatus = mCECAdapter->GetDevicePowerStatus(logical_addres);
                s = cString::sprintf("%s\n   %d# %-15.15s@%04x %-15.15s %-14.14s %-15.15s %-15.15s", *s,
                        logical_addres,
                        mCECAdapter->ToString(logical_addres),
                        phaddr, name.name,
                        mCECAdapter->GetDeviceOSDName(logical_addres).name,
                        mCECAdapter->ToString(vendor),
                        mCECAdapter->ToString(powerstatus));
            }
        }
    }
    return s;
}

/*
 * TEXTVIEWON CEC command.
 */
bool cCECRemote::TextViewOn(cec_logical_address address)
{
    cec_command data;
    if (mCECAdapter == NULL) {
        Esyslog ("TextViewOn CEC Adapter disconnected");
        return false;
    }

    cec_command::Format(data, mCECConfig.baseDevice, address, CEC_OPCODE_TEXT_VIEW_ON);
    Dsyslog("Text View on : %02x %02x %02x", data.initiator, data.destination, data.opcode);
    return mCECAdapter->Transmit(data);
}

/*
 * Try to get the logical address for a device. If specified, try to use
 * the physical address,
 */
cec_logical_address cCECRemote::getLogical(cCECDevice &dev)
{
    if (mCECAdapter == NULL) {
        Esyslog ("getLogical CEC Adapter disconnected");
        return CECDEVICE_UNKNOWN;
    }
    cec_logical_address found = CECDEVICE_UNKNOWN;
    // Logical address already available
    if (dev.mLogicalAddressUsed != CECDEVICE_UNKNOWN) {
        return dev.mLogicalAddressUsed;
    }
    // Try to get logical address from physical address.
    // It may be possible that more than one logical address is available
    // at a physical address!
    if (dev.mPhysicalAddress != 0) {
        cec_logical_addresses devices = mCECAdapter->GetActiveDevices();
        for (int j = 0; j < 16; j++)
        {
            if (devices[j])
            {
                cec_logical_address logical_addres = (cec_logical_address)j;
                if (dev.mPhysicalAddress ==
                        mCECAdapter->GetDevicePhysicalAddress(logical_addres)) {
                    dev.mLogicalAddressUsed = logical_addres;
                    Dsyslog ("Mapping Physical %04x->Logical %d",
                            dev.mPhysicalAddress, logical_addres);
                    found = logical_addres;
                    // Exact match
                    if (dev.mLogicalAddressDefined == logical_addres) {
                        return logical_addres;
                    }
                }
            }
        }
    }
    if (found != CECDEVICE_UNKNOWN) {
        return found;
    }

    // No mapping available, so try as last attempt the defined logical address,
    // if available.
    if (dev.mLogicalAddressDefined == CECDEVICE_UNKNOWN) {
        Esyslog("No fallback logical address for %04x configured", dev.mPhysicalAddress);
        return CECDEVICE_UNKNOWN;
    }
    // Ensure that we don't send accidentally to the own VDR address.

    cec_logical_addresses own = mCECAdapter->GetLogicalAddresses();
    if (own[dev.mLogicalAddressDefined]) {
        Esyslog("Logical address of physical %04x is the VDR", dev.mPhysicalAddress);
        return CECDEVICE_UNKNOWN;
    }
    // Check if device is available.
    if (!mCECAdapter->PollDevice(dev.mLogicalAddressDefined)) {
        Esyslog("Logical address not available", dev.mLogicalAddressDefined);
        return CECDEVICE_UNKNOWN;
    }

    dev.mLogicalAddressUsed = dev.mLogicalAddressDefined;
    return dev.mLogicalAddressDefined;
}

void cCECRemote::ActionKeyPress(cCECCmd &cmd)
{
    cec_logical_address addr;
    cCECList ceckmap;
    cec_user_control_code ceckey;

    addr = getLogical(cmd.mDevice);
    if (addr != CECDEVICE_UNKNOWN) {
        ceckmap = mPlugin->mKeyMaps.VDRtoCECKey((eKeys)cmd.mVal);

        for (cCECListIterator ci = ceckmap.begin(); ci != ceckmap.end();
                ++ci) {
            ceckey = *ci;
            Dsyslog ("Send Keypress VDR %d - > CEC 0x%02x", cmd.mVal, ceckey);
            if (ceckey != CEC_USER_CONTROL_CODE_UNKNOWN) {
                if (!mCECAdapter->SendKeypress(addr, ceckey, true)) {
                    Esyslog("Keypress to %d %s failed",
                            addr, mCECAdapter->ToString(addr));
                    return;
                }
                cCondWait::SleepMs(50);
                if (!mCECAdapter->SendKeyRelease(addr, true)) {
                    Esyslog("SendKeyRelease to %d %s failed",
                            addr, mCECAdapter->ToString(addr));
                }
            }
        }
    }
}

void cCECRemote::WaitForPowerStatus(cec_logical_address addr, cec_power_status newstatus)
{
    cec_power_status status;
    int cnt = 0;
    cCondWait w;

    do {
        w.Wait(100);
        status = mCECAdapter->GetDevicePowerStatus(addr);
        cnt++;
    } while ((status != newstatus) && (cnt < 50) && (status != CEC_POWER_STATUS_UNKNOWN));
}

/*
 * Special exec which handles the svdrp CONN/DISC which may come
 * from this executed shell script
 */
void cCECRemote::Exec(cCECCmd &execcmd)
{
    cCECCmd cmd;
    Dsyslog("Execute script %s", execcmd.mExec.c_str());
    mInExec = true;
    pid_t pid = fork();
    if (pid < 0) {
        Esyslog("fork failed");
        mInExec = false;
        return;
    }
    else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", execcmd.mExec.c_str(), NULL);
        Esyslog("Exec failed");
        abort();
    }

    do {
        cmd = WaitExec(pid);
        Dsyslog ("(%d) ExecAction %d Val %d",
                 cmd.mSerial, cmd.mCmd, cmd.mVal);
        switch (cmd.mCmd) {
        case CEC_EXIT:
            Dsyslog("cCECRemote script stopped");
            break;
        case CEC_RECONNECT:
            Dsyslog("cCECRemote reconnect");
            Disconnect();
            sleep(1);
            Connect();
            break;
        case CEC_CONNECT:
            Dsyslog("cCECRemote connect");
            Connect();
            break;
        case CEC_DISCONNECT:
            Dsyslog("cCECRemote disconnect");
            Disconnect();
            break;
        default:
            Esyslog("Unexpected action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }
        Dsyslog ("(%d) Action finished", cmd.mSerial);
        if (cmd.mSerial != -1) {
            mProcessedSerial = cmd.mSerial;
            mCmdReady.Signal();
        }
    } while (cmd.mCmd != CEC_EXIT);
    mInExec = false;
}
/*
 * Wait until a command is put into the exec command queue.
 * If a command was received remove it and return the received command.
 */
cCECCmd cCECRemote::WaitExec(pid_t pid)
{
    Dsyslog("WaitExec");
    int stat_loc = 0;
    mExecQueueMutex.Lock();
    while (mExecQueue.empty()) {
        mExecQueueMutex.Unlock();
        if (mExecQueueWait.Wait(250)) {
            Dsyslog("  Signal");
        }
        else {
            if (waitpid (pid, &stat_loc, WNOHANG) == pid) {
                Dsyslog("  Script exit with %d", WEXITSTATUS(stat_loc));
                cCECCmd cmd(CEC_EXIT);
                return cmd;
            }
        }
        mExecQueueMutex.Lock();
    }

    cCECCmd cmd = mExecQueue.front();
    mExecQueue.pop_front();
    mExecQueueMutex.Unlock();
    return cmd;
}
/*
 * Worker thread which processes the command queue and executes the
 * received commands.
 */
void cCECRemote::Action(void)
{
    cCECCmd cmd;
    cCECList ceckmap;
    cec_logical_address addr;
    eKeys k;

    Dsyslog("cCECRemote start worker thread");
    while (Running()) {
        cmd = WaitCmd();
        Dsyslog ("(%d) Action %d Val %d Phys Addr %d Logical %04x %04x",
                 cmd.mSerial,
                 cmd.mCmd, cmd.mVal, cmd.mDevice.mPhysicalAddress,
                 cmd.mDevice.mLogicalAddressDefined,
                 cmd.mDevice.mLogicalAddressUsed);
        switch (cmd.mCmd)
        {
        case CEC_KEYRPRESS:
            if ((cmd.mVal >= 0) && (cmd.mVal <= CEC_USER_CONTROL_CODE_MAX)) {
                const cKeyList &inputKeys =
                        mPlugin->mKeyMaps.CECtoVDRKey((cec_user_control_code)cmd.mVal);
                cKeyListIterator ikeys;
                for (ikeys = inputKeys.begin(); ikeys != inputKeys.end(); ++ikeys) {
                    k = *ikeys;
                    Put(k);
                    Dsyslog ("   Put(%d)", k);
                }
            }
            break;
        case CEC_POWERON:
            if (mCECAdapter != NULL) {
                Dsyslog("Power on");
                addr = getLogical(cmd.mDevice);
                if ((addr != CECDEVICE_UNKNOWN) &&
                    (!mCECAdapter->PowerOnDevices(addr))) {
                    Esyslog("PowerOnDevice failed for %s",
                            mCECAdapter->ToString(addr));
                }
                else {
                    WaitForPowerStatus(addr, CEC_POWER_STATUS_ON);
                }
            }
            else {
                Esyslog("PowerOnDevice ignored");
            }
            break;
        case CEC_POWEROFF:
            if (mCECAdapter != NULL) {
                addr = getLogical(cmd.mDevice);
                if ((addr != CECDEVICE_UNKNOWN) &&
                    (!mCECAdapter->StandbyDevices(addr))) {
                    Esyslog("StandbyDevices failed for %s",
                            mCECAdapter->ToString(addr));
                }
                else {
                    WaitForPowerStatus(addr, CEC_POWER_STATUS_STANDBY);
                }
            }
            else {
                Esyslog("StandbyDevices ignored");
            }
            break;
        case CEC_TEXTVIEWON:
            if (mCECAdapter != NULL) {
                Dsyslog("Textviewon");
                addr = getLogical(cmd.mDevice);
                if ((addr != CECDEVICE_UNKNOWN) &&
                    (TextViewOn(addr) != 0)) {
                    Esyslog("TextViewOn failed for %s",
                            mCECAdapter->ToString(addr));
                }
            }
            else {
                Esyslog("Textviewon ignored");
            }
            break;
        case CEC_MAKEACTIVE:
            if (mCECAdapter != NULL) {
                Dsyslog ("Make active");
                if (!mCECAdapter->SetActiveSource()) {
                    Esyslog("SetActiveSource failed");
                }
            }
            else {
                Esyslog("SetActiveSource ignored");
            }
            break;
        case CEC_MAKEINACTIVE:
            if (mCECAdapter != NULL) {
                Dsyslog ("Make inactive");
                if (!mCECAdapter->SetInactiveView()) {
                    Esyslog("SetInactiveView failed");
                }
            }
            else {
                Esyslog("SetInactiveView ignored");
            }
            break;
        case CEC_VDRKEYPRESS:
            if (mCECAdapter != NULL) {
                ActionKeyPress(cmd);
            }
            else {
                Esyslog("Keypress ignored");
            }
            break;
        case CEC_EXECSHELL:
            Dsyslog ("Exec: %s", cmd.mExec.c_str());
            Exec(cmd);
            break;
        case CEC_EXIT:
            Dsyslog("cCECRemote exit worker thread");
            Cancel(0);
            break;
        case CEC_RECONNECT:
            Dsyslog("cCECRemote reconnect");
            Disconnect();
            sleep(1);
            Connect();
            break;
        case CEC_CONNECT:
            Dsyslog("cCECRemote connect");
            Connect();
            break;
        case CEC_DISCONNECT:
            Dsyslog("cCECRemote disconnect");
            Disconnect();
            break;
        case CEC_ACTIVE_SOURCE:
            Dsyslog("cCECRemote active source %d", cmd.mVal);
            break;
        case CEC_EXECTOGGLE:
            Dsyslog("cCECRemote exec_toggle");
            ExecToggle(cmd.mDevice, cmd.mPoweron, cmd.mPoweroff);
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }
        Dsyslog ("(%d) Action finished", cmd.mSerial);
        if (cmd.mSerial != -1) {
            mProcessedSerial = cmd.mSerial;
            mCmdReady.Signal();
        }
    }
    Dsyslog("cCECRemote stop worker thread");
}

/*
 * Get the device power state and execute either the poweron or
 * poweroff command queue.
 */
void cCECRemote::ExecToggle(cCECDevice dev,
                            const cCmdQueue &poweron, const cCmdQueue &poweroff)
{
    bool repeat;
    cec_power_status status;
    cec_logical_address addr;
    cCondWait w;

    if (mCECAdapter == NULL) {
        Esyslog ("ExecToggle CEC Adapter disconnected");
        return;
    }

    addr = getLogical(dev);
    if (addr == CECDEVICE_UNKNOWN) {
        status = CEC_POWER_STATUS_UNKNOWN;
    }
    else
    {
        do {
            repeat = false;
            status = mCECAdapter->GetDevicePowerStatus(addr);
            Dsyslog("ExecToggle: %s", mCECAdapter->ToString(status));
            // If currently in any transition state, wait.
            if ((status == CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON) ||
                    (status == CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY)) {
                w.Wait(1000);
                repeat = true;
            }
        } while (repeat);
    }

    if (status == CEC_POWER_STATUS_ON) {
        PushCmdQueue(poweroff);
    }
    else {
        PushCmdQueue(poweron);
    }
}

/*
 * Put a complete command queue into the worker command queue for execution.
 */
void cCECRemote::PushCmdQueue(const cCmdQueue &cmdList)
{
    if (mCECAdapter == NULL) {
        Esyslog ("PushCmdQueue CEC Adapter disconnected");
        return;
    }
    mWorkerQueueMutex.Lock();
    for (cCmdQueueIterator i = cmdList.begin();
           i != cmdList.end(); i++) {
        mWorkerQueue.push_back(*i);
    }
    mWorkerQueueMutex.Unlock();
    mWorkerQueueWait.Signal();
}

/*
 * Put a command into the worker command queue for execution.
 */
void cCECRemote::PushCmd(const cCECCmd &cmd)
{
    Dsyslog("cCECRemote::PushCmd %d (size %d)", cmd.mCmd, mWorkerQueue.size());

    mWorkerQueueMutex.Lock();
    mWorkerQueue.push_back(cmd);
    mWorkerQueueMutex.Unlock();
    mWorkerQueueWait.Signal();
}

/*
 * Put a command into the worker command queue and wait for execution.
 */
void cCECRemote::PushWaitCmd(cCECCmd &cmd, int timeout)
{
    int serial = cmd.getSerial();
    cmd.mSerial = serial;
    bool signaled = false;
    Dsyslog("cCECRemote::PushWaitCmd %d ID %d (WQ %d EQ %d)",
            cmd.mCmd, serial, mWorkerQueue.size(), mExecQueue.size());

    // Special handling for CEC_CONNECT and CEC_DISCONNECT when called
    // from exec state (used for out of band processing of svdrp commands
    // coming from a script, executed by a command queue.
    if (((cmd.mCmd == CEC_CONNECT) || (cmd.mCmd == CEC_DISCONNECT)) && mInExec){
        Dsyslog("ExecQueue");
        mExecQueueMutex.Lock();
        mExecQueue.push_back(cmd);
        mExecQueueMutex.Unlock();
        mExecQueueWait.Signal();
    }
    // Normal handling
    else {
        mWorkerQueueMutex.Lock();
        mWorkerQueue.push_back(cmd);
        mWorkerQueueMutex.Unlock();
        mWorkerQueueWait.Signal();
    }

    // Wait until this command is processed.
    do {
        signaled = mCmdReady.Wait(timeout);
    } while ((mProcessedSerial != serial) && (signaled));
    if (!signaled) {
        Esyslog("cCECRemote::PushWaitCmd timeout %d %d", mProcessedSerial, serial);
    }
    else {
        Dsyslog("cCECRemote %d %d", mProcessedSerial, serial);
    }
}

/*
 * Wait until a command is put into the worker command queue.
 * If a command was received remove it and return the received command.
 */
cCECCmd cCECRemote::WaitCmd(int timeout)
{
    Dsyslog("Wait");
    mWorkerQueueMutex.Lock();
    while (mWorkerQueue.empty()) {
        mWorkerQueueMutex.Unlock();
        if (mWorkerQueueWait.Wait(timeout)) {
            Dsyslog("  Signal");
        }
        mWorkerQueueMutex.Lock();
    }

    cCECCmd cmd = mWorkerQueue.front();
    mWorkerQueue.pop_front();
    mWorkerQueueMutex.Unlock();

    return cmd;
}

void cCECRemote::Reconnect()
{
    Dsyslog("cCECRemote::Reconnect");
    cCECCmd cmd(CEC_RECONNECT);
    // coming from a script, executed by a command queue.
    if (mInExec) {
        mExecQueueMutex.Lock();
        mExecQueue.push_front(cmd); // Ensure that command is executed ASAP.
        mExecQueueMutex.Unlock();
        mExecQueueWait.Signal();
    }
    else {
        mWorkerQueueMutex.Lock();
        mWorkerQueue.push_front(cmd); // Ensure that command is executed ASAP.
        mWorkerQueueMutex.Unlock();
        mWorkerQueueWait.Signal();
    }
}
