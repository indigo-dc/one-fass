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

#include "Terminator.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <pthread.h>
#include <errno.h>
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
#include "FassDb.h"
#include "FassLog.h"
#include "BasicPlugin.h"
#include "VMPool.h"
#include "User.h"
#include "ObjectXML.h"
#include "VMObject.h"

Terminator::Terminator(
          const string _one_xmlrpc,
          const string _one_secret,
          int64_t _message_size,
          int _timeout,
          int _manager_timer,
          const vector<string> _users,
          int64_t _ttl,
          int64_t _max_wait):
                Manager(_manager_timer),
                one_xmlrpc(_one_xmlrpc),
                one_secret(_one_secret),
                message_size(_message_size),
                timeout(_timeout),
                users(_users),
                ttl(_ttl),
                max_wait(_max_wait) {
    // initialize XML-RPC Client
    ostringstream oss;

    try {
    XMLRPCClient::initialize(one_secret, one_xmlrpc, message_size, timeout);

    client = XMLRPCClient::client();

    oss << "XML-RPC client using "
        << (XMLRPCClient::client())->get_message_size()
    // << " bytes for response buffer." << endl;
    << " bytes for response buffer.";

    FassLog::log("TERMIN", Log::DEBUG, oss);
    }
    catch(runtime_error &) {
        throw;
    }

    // creates the VM pool
    vmpool = new VMPool(client);
}

extern "C" void * tm_loop(void *arg) {
    Terminator * tm;

    if ( arg == 0 ) {
        return 0;
    }

    FassLog::log("TERMIN", Log::INFO, "...started.");

    tm = static_cast<Terminator *>(arg);

    // set thread cancel state
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    // here the actual loop happens
    int rc;
    struct timespec timeout;

    ostringstream oss;

    // actual loop
    while (!tm->stop_manager) {
        bool wait = true;
        timeout.tv_sec  = time(NULL) + tm->manager_timer;
        timeout.tv_nsec = 0;

        tm->lock();
        while (wait && !tm->stop_manager) {  // block for manager_timer seconds
                rc = pthread_cond_timedwait(&tm->cond, &tm->mutex, &timeout);

                if ( rc == ETIMEDOUT ) wait = false;
        }

        tm->unlock();

        // REAL LOOP
        FassLog::log("TERMIN", Log::INFO, "TERMINATOR LOOP:");

        // get DB instance from Fass
        FassDb *database = Fass::instance().fass_db();
        int64_t timestamp = static_cast<int64_t>(time(NULL));
        int rc;
        // get the pool of running VMs
        // rc = tm->get_running();

        // if ( rc != 0 ) {
        //    FassLog::log("TERMIN", Log::ERROR, "Cannot get the VM pool!");
        // }

        // loop over users
        for (vector<string>::const_iterator i= tm->users.begin();
                                      i != tm->users.end(); i++) {
            ostringstream oss;
            vector< string > tokens;
            boost::split(tokens, (*i), boost::is_any_of(":"));
            if ( 4 != tokens.size() ) {
                throw;
            }
            int uid = boost::lexical_cast<int16_t>(tokens[1]);
            // kill running
            // and return instantaneous resource usage
            float cpu = 0.;
            int mem = 0;
            rc = tm->kill_running(uid, cpu, mem);
            oss << "Error terminating running VMs for user " << uid << " !"
                                                                    << endl;
            if ( rc != 0 ) {
                FassLog::log("TERMIN", Log::ERROR, oss);
            }

            ostringstream tmp;
            tmp << "USER: " << uid << " CPU: " << cpu << " MEM: " << mem;
            FassLog::log("SARA", Log::INFO, tmp);

            ostringstream tag;
            tag << "user=" << uid;
            ostringstream c;
            c << cpu;
            ostringstream m;
            m << mem;
            database->write("cpu", c.str(), tag.str(), timestamp);
            database->write("memory", m.str(), tag.str(), timestamp);
            // this should be done in the PM to avoid inconsistencies
            // oss.str("");
            // oss.clear();
            // rc = tm->kill_pending(uid);
            // oss << "Error terminating pending VMs for user " << uid << " !"
            //                                                        << endl;
            // if ( rc != 0 ) {
            //    FassLog::log("TERMIN", Log::ERROR, oss);
            // }
        }
    }

    FassLog::log("TERMIN", Log::INFO, "Terminator stopped.");

    return 0;
}

bool Terminator::terminate(int oid) {
    ostringstream   oss;
    oss << "Terminating VM " << oid;
    FassLog::log("TERMIN", Log::DEBUG, oss);

    try {
        xmlrpc_c::value result;
        xmlrpc_c::paramList plist;
        plist.add(xmlrpc_c::value_int(oid));
        plist.add(xmlrpc_c::value_int(3));  // delete
        client->call("one.vm.recover", plist, &result);
        return true;
    }
    catch (exception const& e) {
        ostringstream   oss;
        oss << "Exception raised: " << e.what();
        FassLog::log("TERMIN", Log::ERROR, oss);
        return false;
    }
}

int Terminator::kill_running(int uid, float& cpu, int& memory) {
    FassLog::log("TERMIN", Log::DEBUG, "Retrieving running VMs from ONE...");
    // this procedure should be locked,
    // or the PM could try to overwrite the vmpool object
    lock();

    int rc;
    ostringstream oss;
    // cleans the cache and gets the VM pool
    rc = vmpool->get_running(uid);
    // TODO(svallero): real return code
    if ( rc != 0 ) {
        oss << "Cannot get the VM pool for user " << uid << " !" << endl;
        FassLog::log("TERMIN", Log::ERROR, oss);
        return rc;
    }
    // loop over VMs
    VMObject * vm;
    const map<int, VMObject*> vms = vmpool->get_objects();
    map<int, VMObject*>::const_iterator  vm_it;
    int count = 0;
    // istantaneous cpu and memory usage (for DB)
    for (vm_it=vms.begin(); vm_it != vms.end(); vm_it++) {
        ostringstream oss;
        vm = static_cast<VMObject*>(vm_it->second);
        int oid = vm->get_oid();
        int static_vm = vm->get_static_vm();
        float cpu_vm = 0.;
        int mem_vm = 0;
        vm->get_requirements(cpu_vm, mem_vm);
        cpu += cpu_vm;
        memory += mem_vm;
        int64_t start = vm->get_start();
        if (!start) continue;  // it can happen
        int64_t stop = static_cast<int64_t>(time(NULL));
        int64_t life = stop - start;
        oss << "OID: " << oid << " LIFETIME: " << life << " TTL: " << ttl << " VM Static: " << static_vm;
        FassLog::log("TERMIN", Log::DDEBUG, oss);
        if (life > ttl && static_vm == 0) {
            terminate(oid);
            count = count + 1;
        }
    }
    unlock();
    oss << "Terminated " << count << " ACTIVE VMs";
    if (uid >= 0) oss << " for user " << uid;
    FassLog::log("TERMIN", Log::INFO, oss);
    return 0;
}

int Terminator::kill_pending(int uid) {
    FassLog::log("TERMIN", Log::DEBUG, "Retrieving pending VMs from ONE...");
    // also this procedure should be locked
    lock();
    int rc;
    ostringstream oss;

    // cleans the cache and gets the VM pool
    rc = vmpool->get_pending(uid);
    // TODO(svallero): real return code
    if ( rc != 0 ) {
        oss << "Cannot get the VM pool for user " << uid << " !" << endl;
        FassLog::log("TERMIN", Log::ERROR, oss);
        return rc;
    }
    // loop over VMs
    VMObject * vm;
    const map<int, VMObject*> vms = vmpool->get_objects();
    map<int, VMObject*>::const_iterator  vm_it;
    int count = 0;
    for (vm_it=vms.begin(); vm_it != vms.end(); vm_it++) {
        ostringstream oss;
        vm = static_cast<VMObject*>(vm_it->second);
        int oid = vm->get_oid();
        int static_vm = vm->get_static_vm();
        int64_t start = vm->get_birth();
        int64_t stop = static_cast<int64_t>(time(NULL));
        int64_t wait = stop - start;
        oss << "OID: " << oid << " WAIT TIME: " << wait << " MAX WAIT : "
            << max_wait << " VM Static: " << static_vm;
        FassLog::log("TERMIN", Log::DDEBUG, oss);
        if (wait > max_wait && static_vm == 0) {
            terminate(oid);
            count = count + 1;
        }
    }
    unlock();
    oss << "Terminated " << count << " PENDING VMs";
    if (uid >= 0) oss << " for user " << uid;
    FassLog::log("TERMIN", Log::INFO, oss);
    return 0;
}
int Terminator::start() {
    pthread_attr_t  pattr;
    ostringstream oss;

    FassLog::log("TERMIN", Log::INFO, "Starting Terminator...");


    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&m_thread, &pattr, tm_loop, reinterpret_cast<void *>(this));

    return true;
}
