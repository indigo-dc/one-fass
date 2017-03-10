/**
 * VMPool.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "VMPool.h"
#include <stdexcept>
#include <iomanip>
#include <vector>

int VMPool::set_up()
{

	int rc;
	ostringstream   oss;
	map<int, VirtualMachine*>::iterator it;

        // Clean the pool to get updated data from OpenNebula

        for (it=objects.begin();it!=objects.end();it++)
        {
            delete it->second;
        }

        objects.clear();


        // Load the ids (to get an updated list of the pool)
        xmlrpc_c::value result;

        rc = load_info(result);

        if ( rc != 0 )
        {
            FassLog::log("VMPOOL",Log::ERROR,
                           "Could not retrieve VM pool info from ONE");
            return -1;
        }


    if ( rc == 0 )
    {

     if (FassLog::log_level() >= Log::DDDEBUG)
      {
         oss << "Virtual Machines and capacity:" << endl;

         oss << right << setw(8)  << "ACTION"    << " "
             << right << setw(8)  << "VM"        << " "
             << right << setw(4)  << "CPU"       << " "
             << right << setw(11) << "Memory"    << " " << endl;
            // << right << setw(3)  << "PCI"       << " "
            // << right << setw(11) << "System DS" << " "
            // << " Image DS" << endl
            // << setw(60) << setfill('-') << "-" << setfill(' ') << endl;
         map<int, VirtualMachine*>::iterator iter;
	 //std::vector<xmlNodePtr>::iterator it;

            for (iter = objects.begin(); iter != objects.end(); iter++)
            {
                int cpu, mem;
                //long long disk;
                //vector<VectorAttribute *> pci;

                string action = "DEPLOY";

                VirtualMachine * vm = static_cast<VirtualMachine *>(iter->second);

                vm->get_requirements(cpu, mem); //, disk, pci);

                if (vm->is_resched())
                {
                    action = "RESCHED";
                }
                else if (vm->is_resume())
                {
                    action = "RESUME";
                }

                oss << right << setw(8)  << action      << " "
                    << right << setw(8)  << it->first   << " "
                    << right << setw(4)  << cpu         << " "
                    << right << setw(11) << mem         << " ";
                    //<< right << setw(3)  << pci.size()  << " "
                    //<< right << setw(11) << disk        << " ";

                oss << endl;
            }
        }
        else
        {
            oss << "Found " << objects.size() << " pending/rescheduling VMs.";
        }

        FassLog::log("VMPool",Log::DEBUG,oss);
    }

    return rc;
}

int VMPool::load_info(xmlrpc_c::value &result)
{
    try
    {
        client->call("one.vmpool.info", "iiii", &result, -2, -1, -1, -1);

        return 0;
    }
    catch (exception const& e)
    {
        ostringstream   oss;
        oss << "Exception raised: " << e.what();

        FassLog::log("VMPool", Log::ERROR, oss);

        return -1;
    }
}


int VMPool::dispatch(int vid, int hid, int dsid, bool resched) const
{
    xmlrpc_c::value deploy_result;

    VirtualMachine* vm = get(vid);

    if (vm != 0) // && vm->clear_log())
    {
        update(vm);
    }

    try
    {
        if (resched == true)
        {
            client->call("one.vm.migrate",// methodName
                         "iibb",          // arguments format
                         &deploy_result,  // resultP
                         vid,             // argument 1 (VM)
                         hid,             // argument 2 (HOST)
                         live_resched,    // argument 3 (LIVE)
                         false);          // argument 4 (ENFORCE)
        }
        else
        {
            client->call("one.vm.deploy", // methodName
                         "iibi",          // arguments format
                         &deploy_result,  // resultP
                         vid,             // argument 1 (VM)
                         hid,             // argument 2 (HOST)
                         false,           // argument 3 (ENFORCE)
                         dsid);           // argument 5 (SYSTEM SD)
        }
    }
    catch (exception const& e)
    {
        ostringstream   oss;

        oss << "Exception raised: " << e.what() << '\n';

        FassLog::log("VM",Log::ERROR,oss);

        return -1;
    }


    return 0;
}


int VMPool::update(int vid) const //, const string &st) const
{
    xmlrpc_c::value result;

    try
    {
        client->call("one.vm.update", "is", &result, vid); //, st.c_str());
    }
    catch (exception const& e)
    {
        return -1;
    }
    
	return 0;
}
