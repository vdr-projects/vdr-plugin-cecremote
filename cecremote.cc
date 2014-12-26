/*
 * cecremote.cc
 *
 *  Created on: 25.12.2014
 *      Author: uli
 */
#include "cecremote.h"


cCECRemote::cCECRemote(void): cRemote("CEC"), cThread("CEC receiver")
{
    Start();
    dsyslog("cCECRemote start");
}


cCECRemote::~cCECRemote()
{
    Cancel(3);
}

void cCECRemote::Action(void)
{
    while (Running()) {
        Put(k9);
        sleep(15);
    }
}
