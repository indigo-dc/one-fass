/**
 * PriorityManager.h 
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */


#ifndef PRIORITY_MANAGER_H_
#define PRIORITY_MANAGER_H_

#include "VMPool.h"
//#include "InitShares.h"
#include "XMLRPCClient.h"
#include <time.h>
#include <pthread.h>

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
        unsigned int _max_vm,
	//list<user> _list_of_users,
        int _manager_timer);

	~PriorityManager(){
            delete client;
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
        pthread_mutex_t mutex;
        pthread_cond_t  cond;

	string one_xmlrpc;
	string one_secret;
	int message_size;
	int timeout;

	unsigned int max_vm;
	//list<user> list_of_users;	
  int manager_timer; 
  bool stop_manager;
  
	bool set_up_pools();
	void do_prioritize();

  XMLRPCClient *client;
	int get_queue();

};

#endif
