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

#include "BasicPlugin.h"
#include "PriorityManager.h"

float BasicPlugin::update_prio(int oid, int uid, int gid,
                              int vm_cpu, int vm_memory,
                              list<user> list_of_users) {
    // Insert algorithm for recomputation of vm_prio here
    // This is a dummy value for testing purposes (to be removed)
    float vm_prio = oid; // revert order
    // float vm_prio = 1./oid * 100000.; // same order
  
    return vm_prio; 

}


