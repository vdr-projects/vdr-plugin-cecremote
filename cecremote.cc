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

    Dsyslog("key pressed %d (%d)", key.keycode, key.duration);
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
    Dsyslog("CEC Command %d : %s", command.opcode,
                                   rem->mCECAdapter->ToString(command.opcode));
    return 0;
}


/*
 * Callback function for libCEC alert messages.
 * Currently only used for debugging.
 */
static int CecAlertCallback(void *cbParam, const libcec_alert type,
                            const libcec_parameter param)
{
    Dsyslog("CecAlert %d)", type);
    switch (type)
    {
    case CEC_ALERT_CONNECTION_LOST:
        Esyslog("Connection lost");
        break;
    case CEC_ALERT_TV_POLL_FAILED:
        Isyslog("TV Poll failed");
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
 * CEC remote constructor.
 * Initializes libCEC and the connected CEC adaptor.
 */

cCECRemote::cCECRemote(const cCECGlobalOptions &options, cPluginCecremote *plugin):
        cRemote("CEC"),
        cThread("CEC receiver"),
        mDevicesFound(0)
{
    mPlugin = plugin;
    mCECAdapter = NULL;
    mCECLogLevel = options.cec_debug;
    mOnStart = options.mOnStart;
    mOnStop = options.mOnStop;
    mOnManualStart = options.mOnManualStart;
    // Initialize Callbacks
    mCECCallbacks.Clear();
    mCECCallbacks.CBCecLogMessage  = &::CecLogMessageCallback;
    mCECCallbacks.CBCecKeyPress    = &::CecKeyPressCallback;
    mCECCallbacks.CBCecCommand     = &::CecCommandCallback;
    mCECCallbacks.CBCecAlert       = &::CecAlertCallback;
    mCECCallbacks.CBCecSourceActivated = &::CECSourceActivatedCallback;

    // Setup CEC configuration
    mCECConfig.Clear();
    strncpy(mCECConfig.strDeviceName, VDRNAME, sizeof(mCECConfig.strDeviceName));
    mCECConfig.clientVersion       = CEC_CLIENT_VERSION_CURRENT;
    mCECConfig.bActivateSource     = CEC_TRUE;
    mCECConfig.iComboKeyTimeoutMs = options.mComboKeyTimeoutMs;

    // If no <cecdevicetype> is specified in the <global>, set default
    if (options.mDeviceTypes.empty())
    {
        mCECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
    }
    else {
        // Add all device types as specified in <cecdevicetype>
        deviceTypeListIterator idev;
        for (idev = options.mDeviceTypes.begin();
                idev != options.mDeviceTypes.end(); ++idev) {
            cec_device_type t = *idev;
            mCECConfig.deviceTypes.Add(t);
            Dsyslog ("   Add device %d", t);
        }
    }

    // Setup callbacks
    mCECConfig.callbackParam = this;
    mCECConfig.callbacks = &mCECCallbacks;
    Connect();
    if (mCECAdapter == NULL) {
        Esyslog("Can not initialize libcec");
        exit(-1);
    }
    Start();
    Dsyslog("cCECRemote start");

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
}

void cCECRemote::Stop()
{
    int cnt = 0;
    cCondWait w;
    Dsyslog("Executing onStop");
    PushCmdQueue(mOnStop);
    // Wait until the worker queue is empty (but longest 10 seconds)
    while (!mWorkerQueue.empty() && (cnt < 20)) {
       w.Wait(500);
    }
    Dsyslog("onStop OK");
}
/*
 * Destructor stops the worker thread and unloads libCEC.
 */
cCECRemote::~cCECRemote()
{
    int cnt = 0;
    cCondWait w;
    cCECCmd cmd(CEC_EXIT);
    PushCmd(cmd);
    // Wait until the worker queue is empty (but longest 5 seconds)
    while (!mWorkerQueue.empty() && (cnt < 10)) {
        w.Wait(500);
    }
    Cancel(3);
    Disconnect();
}

/*
 * Function to list all active CEC devices.
 */
cString cCECRemote::ListDevices()
{
    cString s = "Available CEC Devices:";
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

            uint16_t phaddr = mCECAdapter->GetDevicePhysicalAddress(logical_addres);
            cec_osd_name name = mCECAdapter->GetDeviceOSDName(logical_addres);
            cec_vendor_id vendor = (cec_vendor_id)
                                            mCECAdapter->GetDeviceVendorId(logical_addres);
            if (own[j]) {
                s = cString::sprintf("%s\n   %d# %-15.15s@%04x %-15.15s %-14.14s %-15.15s", *s,
                                     logical_addres,
                                     mCECAdapter->ToString(logical_addres),
                                     phaddr, name.name,
                                     VDRNAME, VDRNAME);
            }
            else {
                    s = cString::sprintf("%s\n   %d# %-15.15s@%04x %-15.15s %-14.14s %-15.15s", *s,
                                 logical_addres,
                                 mCECAdapter->ToString(logical_addres),
                                 phaddr, name.name,
                                 mCECAdapter->GetDeviceOSDName(logical_addres).name,
                                 mCECAdapter->ToString(vendor));
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
/*
 * Worker thread which processes the command queue and executes the
 * received commands.
 */
void cCECRemote::Action(void)
{
    cCECCmd cmd;
    cCECList ceckmap;
    cec_user_control_code ceckey;
    cec_logical_address addr;
    eKeys k;

    Dsyslog("cCECRemote start worker thread");
    while (Running()) {
        cmd = WaitCmd();
        Dsyslog ("Action %d Val %d Phys Addr %d Logical %04x %04x",
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
            Dsyslog("Power on");
            addr = getLogical(cmd.mDevice);
            if ((addr != CECDEVICE_UNKNOWN) &&
                (mCECAdapter->PowerOnDevices(addr) != 0)) {
                Esyslog("PowerOnDevice failed for %s",
                        mCECAdapter->ToString(addr));
            }
            break;
        case CEC_POWEROFF:
            addr = getLogical(cmd.mDevice);
            if ((addr != CECDEVICE_UNKNOWN) &&
                (mCECAdapter->StandbyDevices(addr) != 0)) {
                Esyslog("PowerOnDevice failed for %s",
                        mCECAdapter->ToString(addr));
            }
            break;
        case CEC_TEXTVIEWON:
            Dsyslog("Textviewon");
            addr = getLogical(cmd.mDevice);
            if ((addr != CECDEVICE_UNKNOWN) &&
                (TextViewOn(addr) != 0)) {
                Esyslog("TextViewOn failed for %s",
                        mCECAdapter->ToString(addr));
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
            break;
        case CEC_EXECSHELL:
            Dsyslog ("Exec: %s", cmd.mExec.c_str());
            if (system(cmd.mExec.c_str()) < 0) {
                Esyslog("Exec failed");
            }
            else {
                Dsyslog("Exec OK");
            }
            break;
        case CEC_EXIT:
            Dsyslog("cCECRemote exit worker thread");
            return;
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
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
    if (mCECAdapter == NULL) {
        Esyslog ("ExecToggle CEC Adapter disconnected");
        return;
    }
    // Wait until queue is empty
    cCondWait w;
    bool full = true;
    while (full) {
        mWorkerQueueMutex.Lock();
        full = !mWorkerQueue.empty();
        mWorkerQueueMutex.Unlock();
        if (full) {
            w.Wait(100);
       }
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
    cMutexLock lock(&mWorkerQueueMutex);
    mWorkerQueueMutex.Lock();
    mWorkerQueue.push_back(cmd);
    mWorkerQueueMutex.Unlock();
    mWorkerQueueWait.Signal();
}

/*
 * Wait until a command is put into the worker command queue.
 * If a command was received remove it and return the received command.
 */
cCECCmd cCECRemote::WaitCmd()
{
    cCECCmd cmd;
    mWorkerQueueMutex.Lock();
    while (mWorkerQueue.empty()) {
        mWorkerQueueMutex.Unlock();
        mWorkerQueueWait.Wait(10000);
        mWorkerQueueMutex.Lock();
    }

    cmd = mWorkerQueue.front();
    mWorkerQueue.pop_front();
    mWorkerQueueMutex.Unlock();

    return cmd;
}
