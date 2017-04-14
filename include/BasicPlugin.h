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

#ifndef Basic_PLUGIN_H_
#define Basic_PLUGIN_H_

#include <vector>
#include <list>

#include "PriorityManager.h"
#include "User.h"

using namespace std;

class BasicPlugin
{
public:
	BasicPlugin(){};
	
	~BasicPlugin(){
            tot_cpu.clear();
            tot_mem.clear();
        };

        // methods that should be implemented in custom plugin
        bool  evaluate_total_usage(list<User> user_list);
	double update_prio(int oid, int uid, int gid,
                         int vm_cpu, int vm_memory,
                         User *user,
                         int debug_flag = 0);

protected:
 
    map<int, int64_t> tot_cpu;
    map<int, int64_t> tot_mem;
};

#endif





