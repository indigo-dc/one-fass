/**
 * Copyright © 2017 INFN Torino - INDIGO-DataCloud
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AcctObject.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

void AcctObject::init_attributes() {
    ostringstream oss;
    oss << "" << endl;
/*
    oss << dump(xml) << endl;
*/
    xpath(oid, "/HISTORY/OID", -1);
    xpath(uid, "/HISTORY/VM/UID", -1);
    xpath(vmid, "/HISTORY/VM/ID", -1);

    // TODO(svallero): valore in template o no? che differenza fa?
    xpath(static_vm, "/HISTORY/VM/USER_TEMPLATE/STATIC_VM", 0);
    xpath(memory, "/HISTORY/VM/TEMPLATE/MEMORY", 0);
    xpath<float>(cpu, "/HISTORY/VM/TEMPLATE/CPU", 0);

    // start/stop times from acct entry, not VM
    int64_t def = -1;
    xpath(start_time, "/HISTORY/STIME", def);
    xpath(stop_time, "/HISTORY/ETIME", def);

    oss << "oid " << oid << endl;
    oss << "uid " << uid << endl;
    oss << "vmid " << vmid << endl;
    oss << "static_vm" << static_vm << endl;
    oss << "memory " << memory << endl;
    oss << "cpu " << cpu << endl;
    oss << "start_time " << start_time << endl;
    oss << "stop_time " << stop_time << endl;

    FassLog::log("AcctObj", Log::DDDEBUG, oss);
}
