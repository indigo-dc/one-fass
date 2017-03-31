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

#include "VMPool.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <assert.h>
#include <stdexcept>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

const string VMPool::make_queue(map<float, int, std::greater<float> > prios) {
    FassLog::log("VMPOOL", Log::DDEBUG, "Creating the queue...");
    ostringstream oss;
    // root node
    string retval;
    string root("VM_POOL");
    // write header
    oss << "<" << root << ">";
    for (std::map<float,int>::iterator it=prios.begin(); it!=prios.end(); ++it) {
         FassLog::log("SARA", Log::DDEBUG, "INSIDE THE LOOP");
         int oid = it->second;
         VirtualMachine *vm = get(oid);
         string node = vm->dump_node();
         // we add the prio value to the XML string
         size_t pos = node.find("<VM>");
         node.erase(0,pos+4);
         string prio_node("<VM><PRIO>"+boost::lexical_cast<std::string>(0.-it->first)+"</PRIO>");  
         node.insert(0,prio_node);
         oss << node; 
    }
    FassLog::log("SARA", Log::DDEBUG, "OUTSIDE THE LOOP");
    // write footer
    oss << "</" << root << ">";
    
    retval= oss.str();    

    FassLog::log("VMPOOL", Log::DDEBUG, oss);
    return retval;
}

void VMPool::add_object(xmlNodePtr node) {
        if ( node == 0 || node->children == 0 || node->children->next == 0 ) {
              FassLog::log("VMPOOL", Log::ERROR,
              "XML Node does not represent a valid Virtual Machine");
              return;
        }

        VirtualMachine* vm = new VirtualMachine(node);

        objects.insert(pair<int, VirtualMachine*>(vm->get_oid(), vm));

        ostringstream   oss;
        oss << "Inserting VM object with ID: " << vm->get_oid();
        FassLog::log("VMPOOL", Log::DDEBUG, oss);
}

void VMPool::flush() {
        map<int, VirtualMachine*>::iterator it;

        for (it = objects.begin(); it != objects.end(); it++) {
            delete it->second;
        }

        objects.clear();

}

int VMPool::set_up() {
        int rc;
        ostringstream   oss;

        // clean the pool to get updated data from OpenNebula
        VMPool::flush();

        // load the complete pool of VMs from OpenNebula
        xmlrpc_c::value result;

        rc = load_vms(result);

        if ( rc != 0 ) {
            FassLog::log("VMPOOL", Log::ERROR,
                         "Could not retrieve VM pool info from ONE");
            return -1;
        }

        // read the response
        vector<xmlrpc_c::value> values =
               xmlrpc_c::value_array(result).vectorValueValue();
        bool   success = xmlrpc_c::value_boolean(values[0]);

        // ONE says failure
        if (!success) {
            string message = xmlrpc_c::value_string(values[1]);

            oss << "Oned returned failure... Error: " << message << "\n";
            FassLog::log("VMPOOL", Log::ERROR, oss);
            return -1;
        }

        // the output of the one.vmpool.info method is always a string
        string vmlist(static_cast<string>(xmlrpc_c::value_string(values[1])));
        // parse the response and select only pending/rescheduling VMs
        xmlInitParser();
        if (xml != 0) {
            xmlFreeDoc(xml);
        }

        if ( ctx != 0) {
            xmlXPathFreeContext(ctx);
        }

        xml_parse(vmlist);
        // get root node context 
        std::vector<xmlNodePtr> nodes;
        int n_nodes;
        n_nodes = get_nodes
        ("/VM_POOL/VM[STATE=1 or ((LCM_STATE=3 or LCM_STATE=16) and RESCHED=1)]",
        nodes);

        oss << "I got " << n_nodes << " pending VMs!";
        FassLog::log("VMPOOL", Log::DEBUG, oss);

         for (unsigned int i = 0 ; i < nodes.size(); i++) {
            VMPool::add_object(nodes[i]);
        }

        free_nodes(nodes);

        // clean global variables that might have been allocated by the parser
        xmlCleanupParser();

    return rc;
}

int VMPool::load_vms(xmlrpc_c::value &result) {
    try {
        xmlrpc_c::paramList plist;
        plist.add(xmlrpc_c::value_int(-2));
        plist.add(xmlrpc_c::value_int(-1));
        plist.add(xmlrpc_c::value_int(-1));
        plist.add(xmlrpc_c::value_int(-1));

        client->call("one.vmpool.info", plist, &result);

        return 0;
    }
    catch (exception const& e) {
        ostringstream   oss;
        oss << "Exception raised: " << e.what();

        FassLog::log("VMPool", Log::ERROR, oss);

        return -1;
    }
}
