/*
 * opcode.h
 *
 *  Created on: 17.04.2016
 *      Author: uli
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
