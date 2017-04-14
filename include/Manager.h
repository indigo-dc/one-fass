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

#ifndef MANAGER_H_
#define MANAGER_H_

#include <pthread.h>

using namespace std;

class Manager
{
public:

    Manager(int _manager_timer){
        manager_timer = _manager_timer;
        stop_manager = false;
    };

    virtual ~Manager(){};	

    // to be defined in child classes
    virtual int start() = 0;

    // common methods
    pthread_t get_thread_id() const{
        return m_thread;
    };


    void finalize(){
        lock();
        stop_manager = true;
        pthread_cond_signal(&cond);
        unlock();
    };

protected: 

    void lock(){
        pthread_mutex_lock(&mutex);
    };

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    };

    pthread_t  m_thread;        

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
    
    int manager_timer;
    bool stop_manager;
};

#endif
