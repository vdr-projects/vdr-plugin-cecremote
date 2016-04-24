/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * opcode.h: Class for converting CEC commands as string to the
 *           corresponding CEC opcode.
 */

#ifndef PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_OPCODE_H_
#define PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_OPCODE_H_

#include <string>
#include <map>
#include <cec.h>
#include <cectypes.h>

namespace cecplugin {

class opcodeMap {
private:
    typedef std::map<std::string, CEC::cec_opcode> mapOp;
    typedef mapOp::iterator mapOpIterator;
    static mapOp *map;

    static void initMap();

public:
    static bool getOpcode(std::string name, CEC::cec_opcode &opcode);
};

} // namespace cecplugin
#endif /* PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_OPCODE_H_ */
