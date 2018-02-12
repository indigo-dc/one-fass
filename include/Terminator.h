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

#ifndef TERMINATOR_H_
#define TERMINATOR_H_

#include "Manager.h"
#include "XMLRPCClient.h"
#include "VMPool.h"

#include <time.h>
#include <pthread.h>
#include <vector>

using namespace std;

extern "C" void * tm_loop(void *arg);

class Terminator: public Manager
{
public:



    Terminator(
    const string _one_xmlrpc,
    const string _one_string,
    int64_t _message_size,
    int _timeout,
    int _manager_timer,
    const vector<string> _users, 
    // int64_t _ttl, 
    int64_t _max_wait,
    const string _action);

    ~Terminator(){
        delete vmpool;
    };	

    int start();
        
    // kill pending VMs for user
    int kill_pending(int uid);
    
    VMPool * vmpool;

private:
       
    friend void * tm_loop(void *arg);

    // kill running/pending VMs
    int kill_running(int uid, int64_t ttl, float& cpu, int& memory);
    int kill_pending();

    // terminate a VM
    bool terminate(int oid);

    // perform action on VM
    bool operate(int oid);

    string one_xmlrpc;
    string one_secret;
    int64_t message_size;
    int timeout;

    vector<string> users;
    // int64_t ttl;
    int64_t max_wait;

    string action;
 
    XMLRPCClient *client;
};

#endif
