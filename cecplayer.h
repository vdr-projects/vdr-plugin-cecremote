/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements a VDR Player which displays a still-picture.
 */

#ifndef CECPLAYER_H_
#define CECPLAYER_H_

#include <vdr/plugin.h>
#include <vdr/status.h>
#include <vdr/player.h>
#include <string>
#include "cecremoteplugin.h"

namespace cecplugin {
// The maximum size of a single frame (up to HDTV 1920x1080):
#define TS_SIZE 188
#define CDMAXFRAMESIZE  (KILOBYTE(1024) / TS_SIZE * TS_SIZE) // multiple of TS_SIZE to avoid breaking up TS packets

class cCECPlayer: public cPlayer {
protected:
    void Activate(bool On);
    void LoadStillPicture (const std::string &filename);
    void DisplayStillPicture (void);

    std::string mStillPic;
    cMutex mPlayerMutex;
    uchar *pStillBuf;
    ssize_t mStillBufLen;
public:
    cCECPlayer(const cCECMenu &config);
    virtual ~cCECPlayer();
};

} // namespace cecplugin

#endif /* CECPLAYER_H_ */
