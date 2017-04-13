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

#include <cmath>
#include "PriorityManager.h"
#include "User.h"

bool BasicPlugin::evaluate_total_usage(list<User> user_list) {
    // clear vectors
    tot_cpu.clear();
    tot_mem.clear();

    // loop over users
    for (list<User>::iterator i = user_list.begin();
                             i != user_list.end(); ++i) {
        // ostringstream oss;
        // oss << "" << endl;
        // oss << "User: " << (*i).userID << endl;
        const map<int, struct Usage> cpu_usage = (*i).get_cpu_usage();
        const map<int, struct Usage> memory_usage = (*i).get_memory_usage();
        // loop over periods
        map<int, struct Usage>::const_iterator usage_it;
        map<int, int64_t>::const_iterator it;
        // vector<int64_t>::iterator count;
        // count = tot_cpu.begin();
        int count = 0;
        for (usage_it = cpu_usage.begin(); usage_it != cpu_usage.end();
                                                               usage_it++) {
            struct Usage cpu = static_cast<struct Usage>(usage_it->second);
            int64_t tmp;
            it = tot_cpu.find(count);
            if (it == tot_cpu.end()) tmp = cpu.usage;
            else
               tmp = it->second+cpu.usage;
            tot_cpu.insert(pair<int, int64_t>(count, tmp));
            // oss << count << " " << tmp << endl;
            count++;
        }

        count = 0;
        for (usage_it = memory_usage.begin(); usage_it != memory_usage.end();
                                                                   usage_it++) {
            struct Usage mem = static_cast<struct Usage>(usage_it->second);
            int64_t tmp;
            it = tot_mem.find(count);
            if (it == tot_mem.end()) tmp = mem.usage;
            else
               tmp = it->second+mem.usage;
            tot_mem.insert(pair<int, int64_t>(count, tmp));
            // oss << count << " " << tmp << endl;
            count++;
        }

        // FassLog::log("B_PLUGIN", Log::DDEBUG, oss);
    }

    return true;
}

double BasicPlugin::update_prio(int oid, int64_t start, int uid, int gid,
                              int vm_cpu, int vm_memory,
                              User *user,
                              int debug_flag) {
    // Insert algorithm for recomputation of vm_prio here
    ostringstream oss;

    // This is a dummy value for testing purposes (to be removed)
    double vm_prio = 0.;
    if (!debug_flag) {
        vm_prio = oid;  // revert order
        // vm_prio = 1./oid * 100000.; // same order
        oss << "VM: " << oid << " User: " << uid << " Prio: " << vm_prio;
        FassLog::log("B_PLUGIN", Log::DEBUG, oss);

        return vm_prio;
    }

    // Simplified multi-factor algorithm
    // decay factor
    float decay = 0.5;

    // TODO(svallero): only CPU usage for the time being
    map<int, struct Usage> usage = user->get_cpu_usage();
    map<int, struct Usage>::const_iterator it;
    // half-life usage
    // TODO(svallero): put some check on the entry existence
    it = usage.find(0);
    int64_t h0_usage = (it->second).usage;
    it = usage.find(1);
    int64_t h1_usage = (it->second).usage;
    it = usage.find(2);
    int64_t h2_usage = (it->second).usage;
    it = usage.find(3);
    int64_t h3_usage = (it->second).usage;
    double h_usage = (h0_usage - h1_usage) + decay * (h1_usage - h2_usage)
                                     + decay * decay * (h2_usage -h3_usage);
    // half-life total usage
    map<int, int64_t>::const_iterator t_it;
    t_it = tot_cpu.find(0);
    int64_t t0_usage = t_it->second;
    t_it = tot_cpu.find(1);
    int64_t t1_usage = t_it->second;
    t_it = tot_cpu.find(2);
    int64_t t2_usage = t_it->second;
    t_it = tot_cpu.find(3);
    int64_t t3_usage = t_it->second;
    double t_usage = (t0_usage - t1_usage) + decay * (t1_usage - t2_usage)
                                     + decay * decay * (t2_usage - t3_usage);

    double ratio;
    if (!t_usage) ratio = 0.;
    else
       ratio = static_cast<double>(h_usage)/static_cast<double>(t_usage);

    // this is the fair-share factor F
    vm_prio = pow(2, 0.-(ratio/(user->share)));
    // VMs should not have the same prio value
    // the VM ID is an indication of the start time
    float f = 1./oid;
    vm_prio = vm_prio * f * 1000000000000.;
    // oss << "VM: " << oid << " User: " << uid << " h_usage: " << h_usage
    //                 << " t_usage: " << t_usage << " Prio: " << vm_prio;
    oss << "VM: " << oid << " h0: " << h0_usage
                         << " h1: " << h1_usage
                         << " h2: " << h2_usage
                         << " h3: " << h3_usage
                         << " t0: " << t0_usage
                         << " t1: " << t1_usage
                         << " t2: " << t2_usage
                         << " t3: " << t3_usage << endl;
    oss << "Factor: " << f;
    FassLog::log("B_PLUGIN", Log::DEBUG, oss);

    return vm_prio;
}


