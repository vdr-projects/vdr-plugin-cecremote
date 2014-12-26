/*
 * cecremote.cc
 *
 *  Created on: 25.12.2014
 *      Author: uli
 */
#include "cecremote.h"

#include <iostream>
using namespace std;
#include <cecloader.h>

/*
class CReconnect : public PLATFORM::CThread
{
public:
  static CReconnect& Get(void)
  {
    static CReconnect _instance;
    return _instance;
  }

  virtual ~CReconnect(void) {}

  void* Process(void)
  {
    if (g_parser)
    {
      g_parser->Close();
      if (!g_parser->Open(g_strPort.c_str()))
      {
        PrintToStdOut("Failed to reconnect\n");
        g_bExit = true;
      }
    }
    return NULL;
  }

private:
  CReconnect(void) {}
};
*/

cCECRemote::cCECRemote(void):
        cRemote("CEC"),
        cThread("CEC receiver"),
        mDevicesFound(0),
        mCECAdapter(NULL)
{
  //  Start();
    dsyslog("cCECRemote start");
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
    while (Running()) {
        Put(k9);
        sleep(15);
    }
}

int CecLogMessage(void *cbParam, const cec_log_message message)
{
  //if ((message.level & g_cecLogLevel) == message.level)
  //{
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
    snprintf(strFullLog, 127, "%s[%16ld]\t%s", strLevel.c_str(), message.time, message.message);
    dsyslog(strFullLog);
  //}

  return 0;
}

int CecKeyPress(void *cbParam, const cec_keypress key)
{
    dsyslog("key pressed %d", key.keycode);
  return 0;
}

int CecCommand(void *cbParam, const cec_command command)
{
    dsyslog("CEC Command %d", command.opcode);
  return 0;
}

int CecAlert(void *cbParam, const libcec_alert type, const libcec_parameter param)
{
    dsyslog("CecAlert %d", type);
  switch (type)
  {
  case CEC_ALERT_CONNECTION_LOST:
    /* TODO */
      esyslog("Connection lost - trying to reconnect");

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
    mCECConfig.bActivateSource     = CEC_TRUE;
    mCECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
    mCECConfig.callbackParam = this;
    mCECConfig.callbacks = &mCECCallbacks;
    // Initialize libcec
    mCECAdapter = LibCecInitialise(&mCECConfig);
    if (mCECAdapter == NULL) {
        esyslog("Can not initialize libcec");
        exit(-1);
    }
    // init video on targets that need this
    mCECAdapter->InitVideoStandalone();
    dsyslog("LibCEC %s", mCECAdapter->GetLibInfo());

    mDevicesFound = mCECAdapter->DetectAdapters(mCECAdapterDescription,
                                                MAX_CEC_ADAPTERS, NULL);
    if (mDevicesFound <= 0)
    {
        esyslog("No adapter found");
        UnloadLibCec(mCECAdapter);
        exit(-1);
    }
    for (int i = 0; i < mDevicesFound; i++)
    {
        dsyslog("Device %d path: %s port: %s Firmware %04d", i,
                        mCECAdapterDescription[0].strComPath,
                        mCECAdapterDescription[0].strComName,
                        mCECAdapterDescription[0].iFirmwareVersion);
    }

    if (!mCECAdapter->Open(mCECAdapterDescription[0].strComName))
    {
      esyslog("unable to open the device on port %s",
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
            dsyslog("  %s@%04x %15.15s %15.15s",
                    mCECAdapter->ToString(logical_addres),
                    phaddr, name.name,
                    mCECAdapter->ToString(vendor));
        }
    }

    return false;
}
