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
        	live_resched(_live_resched){
                    xml = 0;
                    ctx = 0;
                };

    ~VMPool(){
    	flush();
        if (ctx != 0) {
            xmlXPathFreeContext(ctx);
        }

        if (xml != 0) {
            xmlFreeDoc(xml);
        }

    };


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

    const string make_queue(map<float, int, std::greater<float> > prios);
protected:

    // deletes pool objects
    void flush();
    // clears XMl node pointers
    void free_nodes(std::vector<xmlNodePtr>& content) const {
        std::vector<xmlNodePtr>::iterator it;

        for (it = content.begin(); it < content.end(); it++) {
        xmlFreeNode(*it);
        }
    };
    // adds an object to the pool
    void add_object(xmlNodePtr node);

    int load_vms(xmlrpc_c::value &result);
    // parse the XML response from ONE into a xmlXPathContextPtr
    bool xml_parse(const string &xml_doc);  
    xmlXPathContextPtr ctx; // XML xpath context pointer
    xmlDocPtr xml;
    

    // get XML nodes corresponding to VMs
    int get_nodes(const string& xpath_expr, std::vector<xmlNodePtr>& content);
    XMLRPCClient *  client;
    unsigned int   machines_limit;
    bool live_resched;

    /**
     * Hash map contains the suitable [id, object] pairs
     */
    map<int, VirtualMachine *> objects; 
};


#endif
