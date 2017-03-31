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

#include "AcctPool.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <assert.h>
#include <stdexcept>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

using namespace std;

void AcctPool::add_object(int uid, xmlNodePtr node) {
        if ( node == 0 || node->children == 0 || node->children->next == 0 ) {
              FassLog::log("AcctPool", Log::ERROR,
              "XML Node does not represent a valid accounting entry");
              return;
        }


        objects.insert(pair<int, xmlNodePtr>(uid, node));

        ostringstream   oss;
        oss << "Inserting accounting object with UID: " << uid;
        FassLog::log("AcctPool", Log::DDEBUG, oss);
        FassLog::log("SARA", Log::INFO, oss);
}

int AcctPool::set_up(list<user> user_list) {
        int rc;
        ostringstream   oss;

        // clean the pool to get updated data from OpenNebula
        objects.clear();

        // load the complete pool of VMs from OpenNebula
        xmlrpc_c::value result;

        rc = load_acct(result);

        if ( rc != 0 ) {
            FassLog::log("AcctPool", Log::ERROR,
                         "Could not retrieve accounting info from ONE");
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
            FassLog::log("AcctPool", Log::ERROR, oss);
            return -1;
        }

        // the output of the one.vmpool.accounting method is always a string
        string acctlist(static_cast<string>(xmlrpc_c::value_string(values[1])));
        // parse the response and group entries per user
        xmlInitParser();
        if (xml != 0) {
            xmlFreeDoc(xml);
        }

        if ( ctx != 0) {
            xmlXPathFreeContext(ctx);
        }

        //FassLog::log("SARA", Log::DEBUG, acctlist);
        xml_parse(acctlist);

        // loop over known users
        // all the nodes for one user should be merged in one pool node
        for (list<user>::const_iterator i = user_list.begin();
                                        i != user_list.end(); ++i) {
            oss.str("");
            oss.clear();
            // get root node context 
            std::vector<xmlNodePtr> u_nodes;
            int n_nodes;
            int uid = (*i).userID; 
            ostringstream search;
            //search << "/HISTORY_RECORDS/HISTORY/VM[UID=" << uid << "]";
            search << "/HISTORY_RECORDS/HISTORY[VM/UID=" << uid << "]";
            n_nodes = get_nodes(search.str(), u_nodes);

            oss << "I got " << n_nodes << " nodes for user " << uid << endl;
            FassLog::log("AcctPool", Log::DEBUG, oss);
            xmlNodePtr node;
            for (unsigned int j = 0 ; j < u_nodes.size(); j++) {
                //ostringstream tmp;
                //tmp << "Processing node " << j << endl;
                //FassLog::log("SARA", Log::DEBUG, tmp );
                if (j == 0) node = xmlCopyNode(u_nodes[j],1);
                else node = xmlTextMerge(node, u_nodes[j]);
            }

            if (n_nodes) {
                AcctPool::add_object(uid, node);
                xmlFreeNode(node);
            }
            free_nodes(u_nodes);
        }

        // clean global variables that might have been allocated by the parser
        xmlCleanupParser();

    return rc;
}

int AcctPool::load_acct(xmlrpc_c::value &result) {
    try {
        xmlrpc_c::paramList plist;
        plist.add(xmlrpc_c::value_int(-2));
        plist.add(xmlrpc_c::value_int(-1));
        plist.add(xmlrpc_c::value_int(-1));

        client->call("one.vmpool.accounting", plist, &result);

        return 0;
    }
    catch (exception const& e) {
        ostringstream   oss;
        oss << "Exception raised: " << e.what();

        FassLog::log("AcctPool", Log::ERROR, oss);

        return -1;
    }
}
