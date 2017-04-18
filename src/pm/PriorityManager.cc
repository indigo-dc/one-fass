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
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "AcctPool.h"
#include "Fass.h"
#include "FassDb.h"
#include "FassLog.h"
#include "BasicPlugin.h"
#include "VMPool.h"
#include "User.h"
#include "ObjectXML.h"
#include "VMObject.h"
#include "Terminator.h"

list<User> PriorityManager::user_list;
map<float, int, std::greater<float> > PriorityManager::priorities;

// unary operator
void PriorityManager::make_user(
          const std::string& user_group_share, const int& sum, User* us) {
  vector< string > tokens;
  boost::split(tokens, user_group_share, boost::is_any_of(":"));

  if ( 4 != tokens.size() ) {
     throw;
  }

  int uid = boost::lexical_cast<int16_t>(tokens[1]);
  int gid = boost::lexical_cast<int16_t>(tokens[2]);
  float share = boost::lexical_cast<float_t>(tokens[3])/sum;

  // PriorityManager::user *us = new PriorityManager::user(uid, gid, share);
  us->userID = uid;
  us->groupID = gid;
  us->share = share;

  // return us;
}

PriorityManager::PriorityManager(
          const string _one_xmlrpc,
          const string _one_secret,
          int64_t _message_size,
          int _timeout,
          // unsigned int _max_vm,
          vector<string> _shares,
          int _manager_timer,
          string _start_time,
          FassDb* _fassdb,
          int _period,
          int _n_periods,
          int _plugin_debug):
                Manager(_manager_timer),
                one_xmlrpc(_one_xmlrpc),
                one_secret(_one_secret),
                message_size(_message_size),
                timeout(_timeout),
                // max_vm(_max_vm),
                shares(_shares),
                // manager_timer(_manager_timer),
                start_time(_start_time),
                queue(""),
                fassdb(_fassdb),
                period(_period),
                n_periods(_n_periods),
                plugin_debug(_plugin_debug) {
    // initialize XML-RPC Client
    ostringstream oss;
    int rc;

    try {
    XMLRPCClient::initialize(one_secret, one_xmlrpc, message_size, timeout);

    client = XMLRPCClient::client();

    oss << "XML-RPC client using "
        << (XMLRPCClient::client())->get_message_size()
    // << " bytes for response buffer." << endl;
    << " bytes for response buffer.";

    FassLog::log("PM", Log::DEBUG, oss);
    }
    catch(runtime_error &) {
        throw;
    }

    // create VM pool
    // bool live_resched = true;
    // vmpool = new VMPool(client, max_vm, live_resched);
    vmpool = new VMPool(client);

    // create list of user objects with initial shares
    rc = calculate_initial_shares();

    if (!rc) {
        FassLog::log("PM", Log::ERROR, "Could not evaluate initial shares.");
    }

    // create the accounting pool
    // XMLRPCClient::initialize(one_secret, one_xmlrpc, message_size, timeout);
    // XMLRPCClient *cli = XMLRPCClient::client();
    // acctpool = new AcctPool(cli);
    acctpool = new AcctPool(client);
}

extern "C" void * pm_loop(void *arg) {
    PriorityManager * pm;

    if ( arg == 0 ) {
        return 0;
    }

    FassLog::log("PM", Log::INFO, "...started.");

    pm = static_cast<PriorityManager *>(arg);

    // set thread cancel state
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    // here the actual loop happens
    int rc;
    struct timespec timeout;

    ostringstream oss;

    // actual loop
    while (!pm->stop_manager) {
        bool wait = true;
        timeout.tv_sec  = time(NULL) + pm->manager_timer;
        timeout.tv_nsec = 0;

        pm->lock();
        while (wait && !pm->stop_manager) {  // block for manager_timer seconds
                rc = pthread_cond_timedwait(&pm->cond, &pm->mutex, &timeout);
                // ostringstream oss;
                // oss << "Timedwait return value: " << rc;
                // FassLog::log("PM",Log::INFO, oss);

                if ( rc == ETIMEDOUT ) wait = false;
        }

        pm->unlock();

        // REAL LOOP
        FassLog::log("PM", Log::INFO, "PRIORITY MANAGER LOOP:");
        // all entries  should be written to DB with the same timestamp
        int64_t timestamp = static_cast<int64_t>(time(NULL));

        int rc;

        // first let's cleanup all the VMs waiting since too long
        Terminator * tm = Fass::instance().terminator(); 
        tm->kill_pending(-1);  

        // let's get the list of pending VMs from ONE
        rc = pm->get_pending();

        if ( rc != 0 ) {
            FassLog::log("PM", Log::ERROR, "Cannot get the VM pool!");
        }

        // get historical usage per user
        pm->historical_usage(timestamp);

        // calculates priorities
        pm->do_prioritize(timestamp);

        // here the queue is actually set
        pm->make_queue();
    }

    FassLog::log("PM", Log::INFO, "Priority Manager stopped.");

    return 0;
}

void PriorityManager::make_queue() {
/*
    ostringstream oss;
    oss << "Reordered:" << endl;
    for (std::map<float,int>::iterator it=priorities.begin(); it!=priorities.end(); ++it) {
        oss << it->second << " " << it->first << endl;
    } 
    FassLog::log("SARA", Log::INFO, oss);
*/
    // here the queue is actually set
    queue = vmpool->make_queue(priorities);

    // cleanup the list of priorities
    priorities.clear();
}

int PriorityManager::start() {
    pthread_attr_t  pattr;
    ostringstream oss;

    FassLog::log("PM", Log::INFO, "Starting Priority Manager...");


    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&m_thread, &pattr, pm_loop, reinterpret_cast<void *>(this));

    return true;
}

void PriorityManager::historical_usage(int64_t timestamp) {
    FassLog::log("PM", Log::INFO, "Evaluating historical usage...");
    int rc;

    // vector of user ids
    vector<int> uids;
    for (list<User>::const_iterator i = user_list.begin();
                                    i != user_list.end(); ++i) {
        uids.push_back((*i).userID);
    }

    // setup time
    // chop the date string
    vector< string > tokens;
    boost::split(tokens, start_time, boost::is_any_of("/"));

    if ( 3 != tokens.size() ) {
      FassLog::log("PM", Log::ERROR, "Wrong format for start_time.");
      throw;
    }

    ostringstream oss;
    int start_day   = boost::lexical_cast<int16_t>(tokens[0]);
    int start_month = boost::lexical_cast<int16_t>(tokens[1]); 
    int start_year  = boost::lexical_cast<int16_t>(tokens[2]);
    oss << "Starting from " << start_day << "/" << start_month 
                                         << "/" << start_year;
    // FassLog::log("PM", Log::DEBUG, oss);

    int64_t time_start = 0;
    struct tm tmp_tm;
    localtime_r(&time_start, &tmp_tm);
    tmp_tm.tm_sec  = 0;
    tmp_tm.tm_min  = 0;
    tmp_tm.tm_hour = 0;
    tmp_tm.tm_mday = start_day;
    tmp_tm.tm_mon  = start_month - 1;
    tmp_tm.tm_year = start_year - 1900;
    tmp_tm.tm_isdst = -1;  // Unknown daylight saving time

    time_start = static_cast<uint64_t>(mktime(&tmp_tm));
    
    // this is to avoid too large xml response from ONE 
    time_start = timestamp - (manager_timer*period*n_periods); 
    // get info from ONE
    rc = acctpool->set_up(uids, time_start);

    if ( rc != 0 ) {
        FassLog::log("PM", Log::ERROR, "Cannot retrieve the accounting info!");
    }

/*
    ostringstream tmp;
    tmp << "" << endl;
    tmp << "start: " << time_start << endl;
    tmp << "stop: " << timestamp << endl;
    FassLog::log("SARA", Log::INFO, tmp);
*/
    // evaluate historical usage
    // user_list, start_time, stop_time, period, n_periods
    int64_t period_s = manager_timer*period;

    rc = acctpool->eval_usage(&user_list, time_start, timestamp,
                                                      period_s, n_periods);

    if ( rc != 0 ) {
        FassLog::log("PM", Log::ERROR, "Cannot evaluate the accounting info!");
    }

     for (list<User>::const_iterator i = user_list.begin();
                                    i != user_list.end(); ++i) {
        // write usage to DB
        rc = fassdb->write_usage((*i));

        if (!rc) {
           FassLog::log("PM", Log::ERROR,
                              "Could not write historical usage  to DB.");
        }
    }
}

int PriorityManager::get_pending() {
    FassLog::log("PM", Log::DEBUG, "Retrieving pending VMs from ONE...");
    int rc;

    // cleans the cache and gets the VM pool
    // lock();
    rc = vmpool->get_pending();
    // unlock();
    // TODO(svallero): real return code
    return rc;
}

void PriorityManager::do_prioritize(int64_t timestamp) {
    FassLog::log("PM", Log::INFO, "Evaluating priorities...");
    // int rc;
    ostringstream oss;

    VMObject * vm;

    int oid;
    int uid;
    int gid;
    int vm_memory;
    int vm_cpu;
    float vm_prio = 1.0;

    // get the VMs previously stored
    const map<int, VMObject*> pending_vms = vmpool->get_objects();

    map<int, VMObject*>::const_iterator  vm_it;

    // the plugin
    BasicPlugin *plugin;
    // TODO(svallero or valzacc): make it a real plugin ;)
    plugin = new BasicPlugin();

    oss << "Found pending VMs:" << endl;

    // neded to normalize historical usage per user
    plugin->evaluate_total_usage(user_list);

    for (vm_it=pending_vms.begin(); vm_it != pending_vms.end(); vm_it++) {
         vm = static_cast<VMObject*>(vm_it->second);

         vm->get_requirements(vm_cpu, vm_memory);
         oid = vm->get_oid();
         uid = vm->get_uid();
         gid = vm->get_gid();

         // TODO(svallero): user_list should be a map
         User user;
         for (list<User>::iterator i = user_list.begin();
                                 i != user_list.end(); ++i) {
              if (((*i).userID == uid)) user = (*i);
         }

         vm_prio = plugin->update_prio(oid, uid, gid, vm_cpu,
                                       vm_memory, &user, plugin_debug);

         priorities.insert(pair<float, int>(vm_prio, oid));

         oss << oid << "(" << vm_prio << ") - ";
         // oss << oid << "(" << priorities[oid] << ") - " ;
    }

    FassLog::log("PM", Log::DDEBUG, oss);

    oss.str("");
    oss.clear();
    oss << "Number of VMs: " << pending_vms.size();
    FassLog::log("PM", Log::DEBUG, oss);

    delete plugin;
    return;
}

bool PriorityManager::calculate_initial_shares() {
    FassLog::log("PM", Log::INFO, "Evaluating initial shares...");

    int rc;
    // TODO(svallero or valzacc):
    // for the time being only user shares are considered
    vector<string> norm_shares;
    int sum = 0;
    for (vector<string>::const_iterator i= shares.begin();
                                      i != shares.end(); i++) {
        vector<string> tokens;
        boost::split(tokens, *i, boost::is_any_of(":"));
        sum += boost::lexical_cast<int16_t>(tokens[3]);
    }

    // all shares should be written to DB with the same timestamp
    int64_t timestamp = static_cast<int64_t>(time(NULL));
    for (vector<string>::const_iterator i= shares.begin();
                                      i != shares.end(); i++) {
       User *us = new User();
       make_user(*i , sum, us);
       user_list.push_back(*us);

       // write share to DB
       rc = fassdb->write_initial_shares(us->share,
                                      boost::lexical_cast<string>(us->userID),
                                      boost::lexical_cast<string>(us->groupID),
                                      timestamp);
       if (!rc) {
           FassLog::log("PM", Log::ERROR,
                              "Could not write initial shares to DB.");
       }

       delete us;
    }

/* 
    ostringstream oss;
    oss << "" << endl;
    for (list<user>::const_iterator i = user_list.begin();
                                    i != user_list.end(); ++i) {
        oss << (*i).userID << " "
        << (*i).groupID << " "
        << (*i).share << endl;
    }

    FassLog::log("SARA", Log::INFO, oss);
*/

    return true;
}


