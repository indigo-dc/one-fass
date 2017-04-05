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

/*
int AcctPool::eval_usage(list<User> user_list, long int time_start,
                                         long int time_stop, 
                                         long int period, 
                                         int n_periods) {
    FassLog::log("AcctPool", Log::INFO, "Setting historical usage per user...");
    // loop over users

    for (list<User>::iterator i = user_list.begin();
                                   i != user_list.end(); ++i) {
    
        int uid = (*i).userID;
        xmlNodePtr node = this->get(uid);    
        //if (node) (*i).set_usage(node);

        //ostringstream oss;
        //oss << "UID ***" << endl;
        //oss << "THIS IS THE NODE" << endl;
        //oss << uid << endl; 
        //FassLog::log("SARA", Log::INFO, oss);

    }

    return 0;
}
*/

void AcctPool::flush() {
    map<int, AcctObject*>::iterator it;
    for (it = objects.begin(); it != objects.end(); it++) {
        delete it->second;
    }

    objects.clear();
}

void AcctPool::make_object(int uid, xmlNodePtr node) {
        if ( node == 0 || node->children == 0 || node->children->next == 0 ) {
              FassLog::log("AcctPool", Log::ERROR,
              "XML Node does not represent a valid accounting entry");
              return;
        }


        AcctObject *acct = new AcctObject(node);

        objects.insert(pair<int, AcctObject*>(uid, acct));

        ostringstream   oss;
        oss << "Creating accounting object with OID: " << acct->get_oid();
        FassLog::log("AcctPool", Log::DDEBUG, oss);

        return;
}

int AcctPool::set_up(vector<int> const &uids) {
        int rc;
        ostringstream   oss;

        // clean the pool to get updated data from OpenNebula
        flush();

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
        if ( xml != 0 ) {
            xmlFreeDoc(xml);
        }

        if ( ctx != 0 ) {
            xmlXPathFreeContext(ctx);
        }

        // FassLog::log("SARA", Log::DEBUG, acctlist);
        xml_parse(acctlist);

        // loop over known users
        // all the nodes for one user should be merged in one pool node
        for (vector<int>::const_iterator i = uids.begin();
                                        i != uids.end(); ++i) {
            oss.str("");
            oss.clear();
            // get root node context
            std::vector<xmlNodePtr> u_nodes;
            int n_nodes;
            int uid = (*i);
            ostringstream search;
            search << "/HISTORY_RECORDS/HISTORY[VM/UID=" << uid << "]";
            n_nodes = get_nodes(search.str(), u_nodes);

            oss << "I got " << n_nodes << " nodes for user " << uid << endl;
            FassLog::log("AcctPool", Log::DEBUG, oss);
            // xmlNodePtr node;
            for (unsigned int j = 0 ; j < u_nodes.size(); j++) {
                // if (j == 0) node = xmlCopyNode(u_nodes[j],1);
                // else node= xmlTextMerge(node, u_nodes[j]);
                make_object(uid, u_nodes[j]);
                if (u_nodes[j]) xmlFreeNode(u_nodes[j]);
            }
            // if (n_nodes) {
            //  add_object(uid, node, time_start, time_stop, period, n_periods);
            //  xmlFreeNode(node);
            // }
            // free_nodes(u_nodes);
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
