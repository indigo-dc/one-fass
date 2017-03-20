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

void VMPool::add_object(xmlNodePtr node) {
        if ( node == 0 || node->children == 0 || node->children->next == 0 ) {
              FassLog::log("VMPOOL", Log::ERROR,
              "XML Node does not represent a valid Virtual Machine");
              return;
        }

        VirtualMachine* vm = new VirtualMachine(node);

        objects.insert(pair<int, VirtualMachine*>(vm->get_oid(), vm));

        ostringstream   oss;
        oss << "Inserting VM object with ID: " << vm->get_oid() << "\n";
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

        // OpenNebula says failure
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
        xml_parse(vmlist);
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

        // free_nodes(nodes);

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

bool VMPool::xml_parse(const string &xml_doc) {
    // copied from ONE ObjectXML
    xmlDocPtr xml = xmlReadMemory(xml_doc.c_str(), xml_doc.length(),
                                  0, 0, XML_PARSE_HUGE);
    if (xml == 0) {
        throw runtime_error("Error parsing XML Document");
        return false;
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0) {
        xmlFreeDoc(xml);
        throw runtime_error("Unable to create new XPath context");
        return false;
    }

    return true;
}

int VMPool::get_nodes(const string& xpath_expr,
                      std::vector<xmlNodePtr>& content) {
    // copied from ONE
    xmlXPathObjectPtr obj;

    obj = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(xpath_expr.c_str()), ctx);

        if (obj == 0) {
        return 0;
    }

    if (obj->nodesetval == 0) {
        xmlXPathFreeObject(obj);
        return 0;
    }

    xmlNodeSetPtr ns = obj->nodesetval;
    int           size = ns->nodeNr;
    int           num_nodes = 0;
    xmlNodePtr    cur;

    for (int i = 0; i < size; ++i) {
        cur = xmlCopyNode(ns->nodeTab[i], 1);

        if ( cur == 0 || cur->type != XML_ELEMENT_NODE ) {
            xmlFreeNode(cur);
            continue;
        }

        content.push_back(cur);
        num_nodes++;
    }

    xmlXPathFreeObject(obj);

    return num_nodes;
}
/*
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
*/
