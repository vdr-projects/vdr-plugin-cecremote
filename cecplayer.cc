/*
 * cecplayer.cc
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#include "cecplayer.h"

using namespace std;

cCECPlayer::cCECPlayer(const string &stillpic) :
                pStillBuf(NULL), mStillBufLen(0)
{
    mStillPic = stillpic;
}

cCECPlayer::~cCECPlayer() {
    if (pStillBuf != NULL) {
        free (pStillBuf);
    }
}

void cCECPlayer::DisplayStillPicture (void)
{
    if (pStillBuf != NULL) {
        DeviceStillPicture((const uchar *)pStillBuf, mStillBufLen);
    }
}

void cCECPlayer::LoadStillPicture (const string &FileName)
{
    int fd;
    ssize_t len;
    int size = CDMAXFRAMESIZE;
    cMutexLock MutexLock(&mPlayerMutex);

    if (pStillBuf != NULL) {
        free (pStillBuf);
    }
    pStillBuf = NULL;
    mStillBufLen = 0;
    fd = open(FileName.c_str(), O_RDONLY);
    if (fd < 0) {
        string errtxt = tr("Can not open still picture: ");
        errtxt += FileName;
        Skins.QueueMessage(mtError, errtxt.c_str());
        esyslog("%s %d Can not open still picture %s",
                __FILE__, __LINE__, FileName.c_str());
        return;
    }

    pStillBuf = (uchar *)malloc (CDMAXFRAMESIZE);
    if (pStillBuf == NULL) {
        esyslog("%s %d Out of memory", __FILE__, __LINE__);
        close(fd);
        return;
    }
    do {
        len = read(fd, &pStillBuf[mStillBufLen], CDMAXFRAMESIZE);
        if (len < 0) {
            esyslog ("%s %d read error %d", __FILE__, __LINE__, errno);
            close(fd);
            free (pStillBuf);
            pStillBuf = NULL;
            mStillBufLen = 0;
            return;
        }
        if (len > 0) {
            mStillBufLen += len;
            if (mStillBufLen >= size) {
                size += CDMAXFRAMESIZE;
                pStillBuf = (uchar *) realloc(pStillBuf, size);
                if (pStillBuf == NULL) {
                    close(fd);
                    mStillBufLen = 0;
                    esyslog("%s %d Out of memory", __FILE__, __LINE__);
                    return;
                }
            }
        }
    } while (len > 0);
    close(fd);
    DisplayStillPicture();
}

void cCECPlayer::Activate(bool On) {
    if (On) {
        LoadStillPicture(mStillPic);
        //Play();
    }
    else {
        //Stop();
    }
}
