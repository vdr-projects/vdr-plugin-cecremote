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

class cCECRemote : public cRemote, private cThread {
private:
    virtual void Action(void);
    //int ReadKey(void);
    //uint64_t ReadKeySequence(void);
    //int MapCodeToFunc(uint64_t Code);
    //void PutKey(uint64_t Code, bool Repeat = false, bool Release = false);
public:
    cCECRemote(void);
    virtual ~cCECRemote();
    virtual bool Initialize(void) {return false;}
};

#endif /* CECREMOTE_H_ */
