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

#include "PriorityManager.h"

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <pthread.h>
#include <errno.h>
// booh #include <time.h>
#include <stdexcept>
#include <cmath>
#include <iomanip>
#include <ctime>
#include <climits>
#include <list>
#include "Fass.h"
#include "FassLog.h"
#include "VMPool.h"
#include "VirtualMachine.h"
// #include "InitShares.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

list<PriorityManager::user*> PriorityManager::user_list;

// unary operator
PriorityManager::user*
PriorityManager::make_user(const std::string& user_group_share, const int& sum) {
  vector< string > tokens;
  boost::split(tokens, user_group_share, boost::is_any_of(":"));

  if ( 4 != tokens.size() ) {
     throw;
  }

  PriorityManager::user *us = new PriorityManager::user(
                      boost::lexical_cast<int16_t>(tokens[1]),
                      boost::lexical_cast<int16_t>(tokens[2]),
                      boost::lexical_cast<float_t>(tokens[3])/sum);

  return us;
}

PriorityManager::PriorityManager(
          const string _one_xmlrpc,
          const string _one_secret,
          int _message_size,
          int _timeout,
          unsigned int _max_vm,
          vector<string> _shares,
          int _manager_timer):
                one_xmlrpc(_one_xmlrpc),
                one_secret(_one_secret),
                message_size(_message_size),
                timeout(_timeout),
                max_vm(_max_vm),
                shares(_shares),
                manager_timer(_manager_timer),
                stop_manager(false) {
    // initialize XML-RPC Client
    ostringstream oss;
    int rc;

    try {
    XMLRPCClient::initialize(one_secret, one_xmlrpc, message_size, timeout);

    client = XMLRPCClient::client();

    oss << "XML-RPC client using "
        << (XMLRPCClient::client())->get_message_size()
    << " bytes for response buffer." << endl;

    FassLog::log("PM", Log::INFO, oss);
    }
    catch(runtime_error &) {
        throw;
    }

    // create list of user objects with initial shares
    rc = calculate_initial_shares();

    if (!rc) {
        FassLog::log("PM", Log::ERROR, "Could not evaluate initial shares.");
    }
}

extern "C" void * pm_loop(void *arg) {
    PriorityManager * pm;

    if ( arg == 0 ) {
        return 0;
    }

    FassLog::log("PM", Log::INFO, "Priority Manager started.");

    pm = static_cast<PriorityManager *>(arg);

    // set thread cancel state
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    // here the actual loop happens
    int rc;
    struct timespec timeout;

    ostringstream oss;
    // oss << "Manager timer is: " << pm->manager_timer;
    // FassLog::log("SARA", Log::INFO, oss);

    // actual loop
    while (!pm->stop_manager) {
        bool wait = true;
        timeout.tv_sec  = time(NULL) + pm->manager_timer;
        timeout.tv_nsec = 0;

        // FassLog::log("SARA", Log::INFO, "Pippo!");

        pm->lock();
        while (wait && !pm->stop_manager) {  // block for manager_timer seconds
                rc = pthread_cond_timedwait(&pm->cond, &pm->mutex, &timeout);
                // ostringstream oss;
                // oss << "Timedwait return value: " << rc;
                // FassLog::log("SARA",Log::INFO, oss);

                if ( rc == ETIMEDOUT ) wait = false;
        }

        // FassLog::log("SARA", Log::INFO, "Pluto!");
        pm->unlock();


        pm->loop();
    }

    FassLog::log("PM", Log::INFO, "Priority Manager stopped.");

    return 0;
}

void PriorityManager::loop() {
    FassLog::log("SARA", Log::INFO, "PRIORITY MANAGER LOOP");

    int rc;
    // let's get the list of pending VMs from ONE
    rc = get_pending();

    if ( rc != 0 ) {
        FassLog::log("PM", Log::ERROR, "Cannot get the VM pool!");
    }

    // return an xml string with reordered VMs
    rc = set_queue();

    if ( ! rc ) {
        FassLog::log("PM", Log::ERROR, "Cannot set the VM pool!");
    }

    // do_prioritize();
}

bool PriorityManager::set_queue() {
    queue = "pippo";

    return true;
}

int PriorityManager::start() {
    pthread_attr_t  pattr;
    ostringstream oss;
    // int rc;

    FassLog::log("PM", Log::INFO, "Starting Priority Manager...");


    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&pm_thread, &pattr, pm_loop, reinterpret_cast<void *>(this));

    return true;
}

int PriorityManager::get_pending() {
    FassLog::log("PM", Log::DEBUG, "Executing get_pending...");
    int rc;
    // VM pool
    // TODO(valzacc or svallero): is it needed?
    bool live_resched = true;
    vmpool = new VMPool(client, max_vm, live_resched);

    // cleans the cache and gets the VM pool
    rc = vmpool->set_up();
    // TODO(svallero): real return code
    return rc;
}

/*
void PriorityManager::do_prioritize() {
    FassLog::log("PM", Log::INFO, "Executing do_prioritize...");
    int rc;
    ostringstream oss;

    VirtualMachine * vm;

    int oid;
    int uid;
    int gid;
    int vm_memory;
    int vm_cpu;
//    list<user> user_list;

    const map<int, VirtualMachine*> pending_vms = vmpool->get_objects();

    time_t time_start = time(0);
    time_t time_end   = time(0);

    tm tmp_tm = *localtime(&time_start);

    int start_month = 1;   // January
    int start_year = 2016; // TODO Make this number settable in the config file 
 
    float vm_prio = 1.0; // value from 0 to 1

    tmp_tm.tm_sec  = 0;
    tmp_tm.tm_min  = 0;
    tmp_tm.tm_hour = 0;
    tmp_tm.tm_mday = 1;
    tmp_tm.tm_mon  = start_month - 1;
    tmp_tm.tm_year = start_year - 1900;
    tmp_tm.tm_isdst = -1; // Unknown daylight saving time

    time_start = mktime(&tmp_tm);

    // first param is the filter flag: 
    // -3: Connected user resources
    // -2: All resources
    // -1: Connected user and his group resources 
    // 0: UID User Resources
 
    //client->call("one.vmpool.accounting", "iii", &result, 0, time_start, time_end); // how to use this info?  TODO with Sara


    map<int, VirtualMachine*>::const_iterator  vm_it;

    oss << "Scheduling Results:" << endl;

    for (vm_it=pending_vms.begin(); vm_it != pending_vms.end(); vm_it++) {

         vm = static_cast<VirtualMachine*>(vm_it->second);
 
         vm->get_requirements(vm_cpu, vm_memory);
         oid = vm->get_oid(); 
         vm->get_uid();
         vm->get_gid();
//         vm->get_state(); 
// // I think that this is not relevant 
// //         vm->get_rank();  
 	       
        //oss << *vm;
        oss << oid << " ";
        //FassLog::log("PM", Log::INFO, oss);
    }
    FassLog::log("PM", Log::INFO, oss);

 
//  oss    << "\tNumber of VMs:            "
//         << pending_vms.size() << endl;

// FassLog::log("PM", Log::INFO, oss);


// //shares retrieved in Fass class and passed into pm 


// PluginBasic::update_prio(oid, uid, gid, vm_cpu, vm_memory, list_of_users, &vm_prio); // TODO we miss historical usage U

  return;
}
*/

bool PriorityManager::calculate_initial_shares() {
    FassLog::log("PM", Log::INFO, "Evaluating initial shares...");

    // TODO: for the time being only user shares are considered
    vector<string> norm_shares;
    int sum=0;
    for (vector<string>::const_iterator i= shares.begin();
                                      i != shares.end(); i++) {
        vector<string> tokens;
        boost::split(tokens, *i, boost::is_any_of(":"));
        sum += boost::lexical_cast<int16_t>(tokens[3]);
    }

    for (vector<string>::const_iterator i= shares.begin();
                                      i != shares.end(); i++) {
       user_list.push_back(make_user(*i , sum));
    }

    ostringstream oss;
    oss << "" << endl;
    for (list<user*>::const_iterator i = user_list.begin(); i != user_list.end(); ++i){
        oss << (*i)->userID << " " << (*i)->groupID << " " << (*i)->share << endl;
    }
 
    FassLog::log("SARA", Log::INFO, oss);

    return true;
}


