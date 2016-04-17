/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the remote receiving and processing the CEC commands.
 */

#include <sys/wait.h>
#include "cecremote.h"
#include "ceclog.h"
#include "cecremoteplugin.h"
#include "ceccontrol.h"

using namespace std;
using namespace cecplugin;

namespace cecplugin {

/*
 * Worker thread which processes the command queue and executes the
 * received commands.
 */
void cCECRemote::Action(void)
{
    cCmd cmd;
    cCECList ceckmap;
    cec_logical_address addr;
    eKeys k;

    Dsyslog("cCECRemote start worker thread");
    while (Running()) {
        cmd = WaitCmd();
        Dsyslog ("(%d) Action %d Val %d Phys Addr %d Logical %04x %04x Op %d",
                 cmd.mSerial,
                 cmd.mCmd, cmd.mVal, cmd.mDevice.mPhysicalAddress,
                 cmd.mDevice.mLogicalAddressDefined,
                 cmd.mDevice.mLogicalAddressUsed,
                 cmd.mCecOpcode);
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
        case CEC_COMMAND:
            Dsyslog("cCECRemote command %d", cmd.mCecOpcode);
            CECCommand(cmd);
            break;
        case CEC_EXECTOGGLE:
            Dsyslog("cCECRemote exec_toggle");
            ExecToggle(cmd.mDevice, cmd.mPoweron, cmd.mPoweroff);
            break;
        default:
            Esyslog("Unknown action %d Val %d", cmd.mCmd, cmd.mVal);
            break;
        }
        Csyslog ("(%d) Action finished", cmd.mSerial);
        if (cmd.mSerial != -1) {
            mProcessedSerial = cmd.mSerial;
            mCmdReady.Signal();
        }
    }
    Dsyslog("cCECRemote stop worker thread");
}

void cCECRemote::ActionKeyPress(cCmd &cmd)
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

// Process actions defined for a CEC command.
void cCECRemote::CECCommand(const cCmd &cmd) {
    mapCommandHandler *h = mPlugin->GetCECCommandHandlers();

    std::pair<mapCommandHandlerIterator, mapCommandHandlerIterator> range;
    range = h->equal_range(cmd.mCecOpcode);

    for (mapCommandHandlerIterator i = range.first; i != range.second; i++) {
        cCECCommandHandler handler = i->second;
        cec_logical_address devaddr = getLogical(handler.mDevice);
        Csyslog("Handler for CEC Command %d test %d %d\n",
                cmd.mCecOpcode, cmd.mCecLogicalAddress, devaddr);
        if (cmd.mCecLogicalAddress == devaddr) {
            Dsyslog("Handler for CEC Command %d found %d %d\n", cmd.mCecOpcode,
                    cmd.mCecLogicalAddress, devaddr);

            // First stop the defined player if running
            if (!handler.mStopMenu.empty()) {
                // Get current running control
                cControl *c = cControl::Control();
                if (c != NULL) {
                    if (cCECControl* cont = dynamic_cast<cCECControl*>(c)) {
                        Dsyslog("Stillpic Player running %s %s",
                                cont->getMenuTitle().c_str(),
                                handler.mStopMenu.c_str());
                        if (cont->getMenuTitle() == handler.mStopMenu) {
                            cont->Shutdown();
                        }
                    }
                }
            }
            // Startup a new menu/player if defined
            if (!handler.mExecMenu.empty()) {
                cCECMenu menuitem;
                if (mPlugin->FindMenu(handler.mExecMenu, menuitem)) {
                    mPlugin->StartPlayer(menuitem);
                }
            }
            // Now Push the command queue
            PushCmdQueue(handler.mCommands);
        }
    }
}

} // namespace cecplugin

