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

#ifndef VM_POOL_H_
#define VM_POOL_H_

#include "VMObject.h"
#include <map>

#include "XMLRPCClient.h"
#include "ObjectXML.h"

using namespace std;

class VMPool: protected ObjectXML 
{
public:

    /* Might be needed in the future 
    VMPool(XMLRPCClient *  _client,
           unsigned int   _machines_limit,
           bool           _live_resched):
		client(_client),
		machines_limit(_machines_limit),
        	live_resched(_live_resched){ */
    VMPool(XMLRPCClient *  _client):
                ObjectXML(),
		client(_client) {
    };

    ~VMPool() {

    	flush();

    };

    // returns the map of VM objects
    const map<int, VMObject*>& get_objects() const {

        return objects;

    };

    // retrieves the pending and rescheduling VMs:
    //    0 on success
    //   -1 on error
    //   -2 if no VMs need to be scheduled
    int get_pending() {
        return set_up("/VM_POOL/VM[STATE=1 or ((LCM_STATE=3 or LCM_STATE=16) and RESCHED=1)]",
                                                                                   "PENDING");
    };

    // retrieves running VMs
    int get_running() {
        // return set_up("/VM_POOL/VM[LCM_STATE=3]", "RUNNING");
        // gets all active states
        return set_up("/VM_POOL/VM[STATE=3]", "RUNNING");
    }; 

    int get_running(int uid) {
        ostringstream search;
        // search << "/VM_POOL/VM[LCM_STATE=3 and UID=" << uid << "]";
        // gets all active states
        search << "/VM_POOL/VM[STATE=3 and UID=" << uid << "]";
        return set_up(search.str(), "RUNNING", uid);
    }; 

    int get_pending(int uid) {
        if (uid == -1) {
            return get_pending();
        } else { 
            ostringstream search;
            search << "/VM_POOL/VM[(STATE=1 or ((LCM_STATE=3 or LCM_STATE=16) and RESCHED=1)) and UID="
                                                                                     << uid << "]";
            return set_up(search.str(), "PENDING", uid);
        }
    }; 

    int set_up(const string search, const string tag, int uid = -1);

    // gets an object from the pool
    VMObject * get(int oid) const
    {

	map<int, VMObject *>::const_iterator it;

        it = objects.find(oid);

        if ( it == objects.end() )
        {
            return 0;
        }
        else
        {
            return static_cast<VMObject *> (it->second);
        }
        
    };

    // creates the ordered queue
    const string make_queue(map<float, int, std::greater<float> > prios);

private:

    // deletes pool objects
    void flush();

    // adds an object to the pool
    void add_object(xmlNodePtr node, const string tag, int uid);
 
    // gets list of VMs from ONE
    int load_vms(xmlrpc_c::value &result);

    // class variables

    XMLRPCClient *  client;
    //unsigned int   machines_limit;
    //bool live_resched;

    // hash map contains the suitable [id, object] pairs
    map<int, VMObject *> objects; 
};


#endif
