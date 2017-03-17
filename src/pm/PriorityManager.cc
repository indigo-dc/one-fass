/**
 * PriorityManager.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "Fass.h"
#include "FassLog.h"
#include "InitShares.h"
#include "PriorityManager.h"
#include "VMPool.h"
#include "VirtualMachine.h"

// booh #include <time.h>
#include <stdexcept>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <pthread.h>
#include <cmath>
#include <iomanip>
#include <ctime>
#include <climits>
#include <errno.h>

using namespace std;

PriorityManager::PriorityManager(
    const string _one_xmlrpc,
    const string _one_secret,
	  int _message_size,
	  int _timeout,
    unsigned int _max_vm,
	  list<user> list_of_users,
    int _manager_timer):

		one_xmlrpc(_one_xmlrpc),
		one_secret(_one_secret),
		message_size(_message_size),
		timeout(_timeout),
    max_vm(_max_vm),
    list_of_users(_list_of_users),
		manager_timer(_manager_timer),
    stop_manager(false)
{
    // initialize XML-RPC Client
    ostringstream oss;
    
    try
    {

    XMLRPCClient::initialize(one_secret, one_xmlrpc, message_size, timeout);

    client = XMLRPCClient::client();

    oss << "XML-RPC client using " << (XMLRPCClient::client())->get_message_size()
    << " bytes for response buffer.\n";

    FassLog::log("PM", Log::INFO, oss);

    }
    catch(runtime_error &)
    {
        throw;
    }

};

extern "C" void * pm_loop(void *arg)
{

    PriorityManager * pm;

    if ( arg == 0 )
    {
        return 0;
    }

    FassLog::log("PM",Log::INFO,"Priority Manager started.");

    pm = static_cast<PriorityManager *>(arg);

    // set thread cancel state
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
 
    // here the actual loop happens
    //pm->loop(pm->manager_timer);
    int rc;
    struct timespec timeout;
    

    ostringstream oss;
    oss << "Manager timer is: " << pm->manager_timer;
    FassLog::log("SARA",Log::INFO, oss);

    // actual loop
    while (!pm->stop_manager)
    {
        bool wait = true;
    	timeout.tv_sec  = time(NULL) + pm->manager_timer;
    	timeout.tv_nsec = 0; 

        pm->lock();
        while ( wait && !pm->stop_manager){ // block for manager_timer seconds
        	rc = pthread_cond_timedwait(&pm->cond,&pm->mutex, &timeout);
    		//ostringstream oss;
    		//oss << "Timedwait return value: " << rc;
    		//FassLog::log("SARA",Log::INFO, oss);

                if ( rc == ETIMEDOUT ) wait = false;
        }

        pm->unlock();

        pm->loop();
    }

    FassLog::log("PM",Log::INFO,"Priority Manager stopped.");
   
    return 0;

};

void PriorityManager::loop(){


    FassLog::log("SARA",Log::INFO,"PRIORITY MANAGER LOOP");

    int rc;
    // let's get the list of pending VMs from ONE
    rc = get_queue();

    if ( rc != 0 )
    {
        FassLog::log("PM",Log::ERROR, "Cannot get the VM pool!");
    }

   
    // do_prioritize();
    
};

int PriorityManager::start()
{
    pthread_attr_t  pattr;
    ostringstream oss;


    FassLog::log("PM",Log::INFO,"Starting Priority Manager...");
    
    pthread_attr_init (&pattr);
    pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);
                                                                                      
    pthread_create(&pm_thread,&pattr,pm_loop,(void *)this);    

    return true;
  
};

int PriorityManager::get_queue()
{
    
    FassLog::log("PM",Log::DEBUG, "Executing get_queue...");
    int rc;
    // VM pool
    bool live_resched = true; // TODO: non sono sicura che ci serva
    vmpool = new VMPool(client, max_vm, live_resched);

    // cleans the cache and gets the VM pool
    rc = vmpool->set_up();
    // TODO: real return code
    return rc;
    
};

/*
void PriorityManager::do_prioritize()
{
 VirtualMachine * vm;
    
    int oid;
    int uid;
    int gid;    
    int vm_memory;
    int vm_cpu;
    list<user> list_of_users;

    const map<int, VirtualMachine*> pending_vms = vmpool->get_objects();
    int rc;


 ostringstream oss;

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

 for (vm_it=pending_vms.begin(); vm_it != pending_vms.end(); vm_it++)
     {
         vm = static_cast<VirtualMachine*>(vm_it->second);
 
         vm->get_requirements(vm_cpu, vm_memory);

	 vm->get_oid(); 
	 vm->get_uid();
	 vm->get_gid();
	 vm->get_state(); 
 // I think that this is not relevant 
 //         vm->get_rank();  
 	       
        oss << *vm;
    }
  FassLog::log("PM", Log::INFO, oss);

 
  oss    << "\tNumber of VMs:            "
         << pending_vms.size() << endl;

 FassLog::log("PM", Log::INFO, oss);


 //shares retrieved in Fass class and passed into pm 


 PluginBasic::update_prio(oid, uid, gid, vm_cpu, vm_memory, list_of_users, &vm_prio); // TODO we miss historical usage U

}
*/

