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

//TODO
//    match_schedule();

//TODO
//    dispatch();

   do_schedule();

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

void PriorityManager::do_schedule()
{
 VirtualMachine * vm;

    int vm_memory;
    int vm_cpu;

    const map<int, VirtualMachine*> pending_vms = vmpool->get_objects();
    int rc;


 ostringstream oss;
 int time_start;
 int time_end; 

// I need start and end
// for start one can use 1st day of current year
// for end current date

// TODO call the plugin basic + save the age from one.vmpool.accounting info


}

/*

void PriorityManager::match_schedule()
{
    VirtualMachineXML * vm;

    int vm_memory;
    int vm_cpu;
    long long vm_disk;
    vector<VectorAttribute *> vm_pci;

    int n_resources;
    int n_matched;
    int n_auth;
    int n_error;
    int n_fits;

    //HostXML * host;
    //DatastoreXML *ds;

    string m_error;

    map<int, ObjectXML*>::const_iterator  vm_it;
    map<int, ObjectXML*>::const_iterator  obj_it;

    vector<SchedulerPolicy *>::iterator it;

    const map<int, ObjectXML*> pending_vms = vmpool->get_objects();
    //const map<int, ObjectXML*> hosts       = hpool->get_objects();
    //const map<int, ObjectXML*> datastores  = dspool->get_objects();
    //const map<int, ObjectXML*> users       = upool->get_objects();

    //double total_cl_match_time = 0;
    //double total_host_match_time = 0;
    //double total_host_rank_time = 0;
    //double total_ds_match_time = 0;
    //double total_ds_rank_time = 0;

    time_t stime = time(0);

    for (vm_it=pending_vms.begin(); vm_it != pending_vms.end(); vm_it++)
    {
        vm = static_cast<VirtualMachineXML*>(vm_it->second);

        vm->get_requirements(vm_cpu, vm_memory, vm_disk, vm_pci);

        n_resources = 0;
        n_fits    = 0;
        n_matched = 0;
        n_auth    = 0;
        n_error   = 0;


    if (FassLog::log_level() >= Log::DDEBUG)
    {
        ostringstream oss;

        oss << "Match Making statistics:\n"
            << "\tNumber of VMs:            "
            << pending_vms.size() << endl
            << "\tTotal time:               "
            << one_util::float_to_str(time(0) - stime) << "s" << endl

        FassLog::log("PM", Log::DDEBUG, oss);
    }

    if (FassLog::log_level() >= Log::DDDEBUG)
    {
        ostringstream oss;

        oss << "Priority Management Results:" << endl;

        for (map<int, ObjectXML*>::const_iterator vm_it=pending_vms.begin();
            vm_it != pending_vms.end(); vm_it++)
        {
            vm = static_cast<VirtualMachineXML*>(vm_it->second);

            oss << *vm;
        }

        FassLog::log("PM", Log::DDDEBUG, oss);
    }
}

*/
/*
void PriorityManager::dispatch()
{
    //HostXML *           host;
    //DatastoreXML *      ds;
    VirtualMachineXML * vm;

    ostringstream dss;
    string        error;

    int cpu, mem;
    long long dsk;
    vector<VectorAttribute *> pci;

    int hid, dsid, cid;

    unsigned int dispatched_vms = 0;
    bool dispatched, matched;
    char * estr;

    map<int, ObjectXML*>::const_iterator vm_it;

    vector<Resource *>::const_reverse_iterator i, j;

    const map<int, ObjectXML*> pending_vms = vmpool->get_objects();

    dss << "Dispatching VMs to hosts:\n" << "\tVMID\tHost\tSystem DS\n"
        << "\t-------------------------\n";

    //--------------------------------------------------------------------------
    // Dispatch each VM till we reach the dispatch limit
    //--------------------------------------------------------------------------

    for (vm_it = pending_vms.begin();
         vm_it != pending_vms.end() &&
            ( dispatch_limit <= 0 || dispatched_vms < dispatch_limit );
         vm_it++)
    {
        dispatched = false;

        vm = static_cast<VirtualMachineXML*>(vm_it->second);

       // const vector<Resource *> resources = vm->get_match_hosts();
*/
/* Let's see what to do with this

int PriorityManager::do_scheduled_actions()
{
    VirtualMachineXML* vm;

    const map<int, ObjectXML*>  vms = vmapool->get_objects();
    map<int, ObjectXML*>::const_iterator vm_it;

    vector<Attribute *> attributes;
    vector<Attribute *>::iterator it;

    VectorAttribute* vatt;

    int action_time;
    int done_time;
    int has_time;
    int has_done;

    string action_st, error_msg;

    time_t the_time = time(0);
    string time_str = one_util::log_time(the_time);

    for (vm_it=vms.begin(); vm_it != vms.end(); vm_it++)
    {
        vm = static_cast<VirtualMachineXML*>(vm_it->second);

        vm->get_actions(attributes);

        // TODO: Sort actions by TIME
        for (it=attributes.begin(); it != attributes.end(); it++)
        {
            vatt = dynamic_cast<VectorAttribute*>(*it);

            if (vatt == 0)
            {
                delete *it;

                continue;
            }

            has_time  = vatt->vector_value("TIME", action_time);
            has_done  = vatt->vector_value("DONE", done_time);
            action_st = vatt->vector_value("ACTION");

            if (has_time == 0 && has_done == -1 && action_time < the_time)
            {
                ostringstream oss;

                int rc = VirtualMachineXML::parse_action_name(action_st);

                oss << "Executing action '" << action_st << "' for VM "
                    << vm->get_oid() << " : ";

                if ( rc != 0 )
                {
                    error_msg = "This action is not supported.";
                }
                else
                {
                    rc = vmapool->action(vm->get_oid(), action_st, error_msg);
                }

                if (rc == 0)
                {
                    vatt->remove("MESSAGE");
                    vatt->replace("DONE", static_cast<int>(the_time));

                    oss << "Success.";
                }
                else
                {
                    ostringstream oss_aux;

                    oss_aux << time_str << " : " << error_msg;

                    vatt->replace("MESSAGE", oss_aux.str());

                    oss << "Failure. " << error_msg;
                }

                NebulaLog::log("VM", Log::INFO, oss);
            }

            vm->set_attribute(vatt);
        }

        vmpool->update(vm);
    }

    return 0;
}
*/
