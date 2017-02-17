/**
 * PriorityManager.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "Fass.h"
#include "FassLog.h"
//#include "PriorityManager.h"
//#include "VMPool.h"
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

using namespace std;

PriorityManager::PriorityManager(
        const string _one_xmlrpc,
	int _message_size,
	int _timeout,
	int _max_vm,
	int _max_dispatch,
	int _live_rescheds):
		one_xmlrpc(_one_xmlrpc),
		message_size(_message_size),
		timeout(_timeout),
		max_vm(_max_vm),
		max_dispatch(_max_dispatch),
		live_rescheds(_live_rescheds)


{

    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, message_size);

};




bool PriorityManager::start()
{
    int rc;

    ifstream      file;
    ostringstream oss;

    string etc_path;
    //unsigned int machines_limit;
    //unsigned int live_rescheds;

    //pthread_attr_t pattr;
    FassLog::log("PM",Log::INFO,"Starting Priority Manager...");
  
    XMLRPCClient * client = XMLRPCClient::client();
 
    // XML-RPC Client
    try
    {

        XMLRPCClient::initialize("", one_xmlrpc, message_size, timeout);

        oss.str("");

        oss << "XML-RPC client using " << (XMLRPCClient::client())->get_message_size()
            << " bytes for response buffer.\n";

        FassLog::log("PM", Log::INFO, oss);

    }
    catch(runtime_error &)
    {
        throw;
    }

       // Get oned configuration
        try
        {
            xmlrpc_c::value result;
            vector<xmlrpc_c::value> values;

            client->call("one.system.config", "", &result);

        }
        catch (exception const& e)
        {
            ostringstream oss;

            oss << "Cannot contact oned, will retry... Error: " << e.what();

            FassLog::log("PM", Log::ERROR, oss);
        }

        FassLog::log("PM", Log::INFO, "oned successfully contacted.");

//TODO
//        do_scheduled_actions();

    rc = set_up_pools();

    if ( rc != 0 )
    {
	//ostringstream oss;

        //oss << "Cannot get the VM pool: " << strerror(errno);
        //FassLog::log("PM",Log::ERROR,oss);
        return false; 
    }
   
 do_prioritize();

return true;

};

bool PriorityManager::set_up_pools()
{
    int rc;
    XMLRPCClient * client = XMLRPCClient::client();
    // VM pool
    vmpool = new VMPool(client, max_vm, live_rescheds==1);

    //Cleans the cache and gets the VM pool
    rc = vmpool->set_up();

    return 0;
};

void PriorityManager::do_prioritize()
{
 VirtualMachine * vm;

    int vm_memory;
    int vm_cpu;

    const map<int, VirtualMachine*> pending_vms = vmpool->get_objects();
    int rc;


 ostringstream oss;

 time_t time_start = time(0);
 time_t time_end   = time(0);

 tm tmp_tm = *localtime(&time_start);

 int start_month = 1;   // January
 int start_year = 2016; // TODO Make this number settable in the Configurator class 


 tmp_tm.tm_sec  = 0;
 tmp_tm.tm_min  = 0;
 tmp_tm.tm_hour = 0;
 tmp_tm.tm_mday = 1;
 tmp_tm.tm_mon  = start_month - 1;
 tmp_tm.tm_year = start_year - 1900;
 tmp_tm.tm_isdst = -1; // Unknown daylight saving time

 time_start = mktime(&tmp_tm);

 client->call("one.vmpool.accounting", "iii", &result, 0, time_start, time_end); // TODO how to use this info? 


 map<int, VirtualMachine*>::const_iterator  vm_it;

 oss << "Scheduling Results:" << endl;

 for (vm_it=pending_vms.begin(); vm_it != pending_vms.end(); vm_it++)
     {
         vm = static_cast<VirtualMachine*>(vm_it->second);
 
         vm->get_requirements(vm_cpu, vm_memory);
	 vm->get_oid(); // TODO add this to VirtualMachine class
	 vm->get_uid();
	 vm->get_gid();
	 vm->get_state(); 
         
        oss << *vm;
    }
  FassLog::log("PM", Log::INFO, oss);

 
  oss    << "\tNumber of VMs:            "
         << pending_vms.size() << endl;

 FassLog::log("PM", Log::INFO, oss);

 PluginBasic(oid, uid, gid, vm_cpu, vm_memory, usage, share, &vm_prio);


}

