/*
 * cecplayer.h
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#ifndef CECPLAYER_H_
#define CECPLAYER_H_

#include <vdr/plugin.h>
#include <vdr/status.h>
#include <vdr/player.h>
#include <string>
#include "cecremoteplugin.h"
// The maximum size of a single frame (up to HDTV 1920x1080):
#define TS_SIZE 188
#define CDMAXFRAMESIZE  (KILOBYTE(1024) / TS_SIZE * TS_SIZE) // multiple of TS_SIZE to avoid breaking up TS packets

class cCECPlayer: public cPlayer {
protected:
    void Activate(bool On);
    void LoadStillPicture (const std::string &filename);
    void DisplayStillPicture (void);

    cCECDevInfo mConfig;
    std::string mStillPic;
    cMutex mPlayerMutex;
    uchar *pStillBuf;
    ssize_t mStillBufLen;
public:
    cCECPlayer(const cCECDevInfo &config);
    virtual ~cCECPlayer();
};

#endif /* CECPLAYER_H_ */
