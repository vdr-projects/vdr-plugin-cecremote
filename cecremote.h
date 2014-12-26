/*
 * cecremote.h
 *
 *  Created on: 25.12.2014
 *      Author: uli
 */

#ifndef CECREMOTE_H_
#define CECREMOTE_H_

#include <vdr/plugin.h>
#include <vdr/remote.h>
#include <vdr/thread.h>
#include <cectypes.h>
#include <cec.h>
#include <stdint.h>

#define MAX_CEC_ADAPTERS 10

using namespace CEC;

class cCECRemote : public cRemote, private cThread {
public:
    cCECRemote(void);
    virtual ~cCECRemote();
    virtual bool Initialize(void);

private:
    uint8_t mDevicesFound;
    ICECAdapter            *mCECAdapter;
    libcec_configuration   mCECConfig;
    ICECCallbacks          mCECCallbacks;
    cec_adapter_descriptor mCECAdapterDescription[MAX_CEC_ADAPTERS];

    virtual void Action(void);
    //int ReadKey(void);
    //uint64_t ReadKeySequence(void);
    //int MapCodeToFunc(uint64_t Code);
    //void PutKey(uint64_t Code, bool Repeat = false, bool Release = false);
};

#endif /* CECREMOTE_H_ */
