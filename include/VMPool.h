/**
 * VMPool.h 
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */



#ifndef VM_POOL_XML_H_
#define VM_POOL_XML_H_

// this should not be necessary #include "PoolXML.h"
#include "VirtualMachine.h"
#include "XMLRPCClient.h"

#include <map>

using namespace std;

class VMPool
{
public:

    VMPool(XMLRPCClient *  _client,
           unsigned int   _machines_limit,
           bool           _live_resched):
		client(_client),
		machines_limit(_machines_limit),
        	live_resched(_live_resched){};

    ~VMPool(){};
    
   const map<int, VirtualMachine*>& get_objects() const
    {
        return objects;
    };


    /**
     * Retrieves the pending and rescheduling VMs
     *
     * @return   0 on success
     *          -1 on error
     *          -2 if no VMs need to be scheduled
     */
    int set_up();

    /**
     *  Gets an object from the pool
     *   @param oid the object unique identifier
     *
     *   @return a pointer to the object, 0 in case of failure
     */
    VirtualMachine * get(int oid) const
    {

	map<int, VirtualMachine *>::const_iterator it;

        it = objects.find(oid);

        if ( it == objects.end() )
        {
            return 0;
        }
        else
        {
            return static_cast<VirtualMachine *> (it->second);
        }
        
    };

    /**
     *  Dispatch a VM to the given host
     *    @param vid the VM id
     *    @param hid the id of the target host
     *    @param resched the machine is going to be rescheduled
     */
    int dispatch(int vid, int hid, int dsid, bool resched) const;

    /**
     *  Update the VM template
     *    @param vid the VM id
     *    @param st the template string
     *
     *    @return 0 on success, -1 otherwise
     */
    int update(int vid) const;//, const string &st) const;

    /**
     *  Update the VM template
     *      @param the VM
     *
     *      @return 0 on success, -1 otherwise
     */
    int update(VirtualMachine * vm) const
    {
        string xml;

        return update(vm->get_oid());//, vm->get_template(xml));
    };

protected:


    virtual int load_info(xmlrpc_c::value &result);

    /**
     * Do live migrations to resched VMs
     */
    bool live_resched;
    XMLRPCClient *  client;
    unsigned int   machines_limit;

    /**
     * Hash map contains the suitable [id, object] pairs
     */
    map<int, VirtualMachine *> objects; 
};


#endif
