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

#ifndef PRIORITY_MANAGER_H_
#define PRIORITY_MANAGER_H_

#include "VMPool.h"
#include "AcctPool.h"
#include "FassDb.h"
#include "XMLRPCClient.h"
#include "User.h"

#include <time.h>
#include <pthread.h>
#include <list>

using namespace std;

extern "C" void * pm_loop(void *arg);

class PriorityManager
{
public:



    PriorityManager(
    const string _one_xmlrpc,
    const string _one_string,
    int _message_size,
    int _timeout,
    // unsigned int _max_vm,
    vector<string> _shares,
    int _manager_timer,
    FassDb* _fassdb);

    ~PriorityManager(){
       
        delete client;
        delete vmpool;
        delete acctpool;
        user_list.clear();

    };	

    pthread_t get_thread_id() const{
        return pm_thread;
    };

    int start();

    void finalize(){

        lock();
        stop_manager = true;
        pthread_cond_signal(&cond);
        unlock();
    };

    VMPool * vmpool;
    AcctPool * acctpool;

    // returns the reordered queue
    string& get_queue() {
        return queue;
    };

private:
       
        friend void * pm_loop(void *arg);
        // we do not need all the ONE ActionManager machinery
        void loop();

        void lock(){
            pthread_mutex_lock(&mutex);
        };

        void unlock()
        {
            pthread_mutex_unlock(&mutex);
        };

        pthread_t  pm_thread;        // thread for the Priority Manager
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

	string one_xmlrpc;
	string one_secret;
	int message_size;
	int timeout;

	// unsigned int max_vm;
	vector<string> shares;	
        int manager_timer; 
        bool stop_manager;
  
        bool calculate_initial_shares();
	bool set_up_pools();
        void historical_usage(long int timestamp);
	void do_prioritize(long int timestamp);

        // the reordered queue
        string queue;
       
        XMLRPCClient *client;
        FassDb *fassdb;
        // gets pending VMs from ONE
	int get_pending();
 
        static list<user> user_list;

        void make_user(const std::string& user_group_share, const int& sum, user* us);
         
        // map<prio, oid> sorted in decreasing prio values
        static map<float, int, std::greater<float> > priorities; 
};

#endif
