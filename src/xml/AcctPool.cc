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

int AcctPool::eval_usage(list<User> *user_list, int64_t &time_start,
                                         int64_t &time_stop,
                                         int64_t &period,
                                         int &num_periods) {
    FassLog::log("AcctPool", Log::DEBUG,
                          "Evaluating historical usage per user...");
    // we need one entry more to evaluate the deltas
    int n_periods = num_periods + 1;
    // loop over users
    for (list<User>::iterator i = user_list->begin();
                                   i != user_list->end(); ++i) {
        int uid = (*i).userID;

        // get list of entries for this user
        list<AcctObject*> acct_list = this->get(uid);
        if ( acct_list.empty() ) {
            ostringstream oss;
            oss << "No accounting entries for user " << uid;
            FassLog::log("AcctPool", Log::DEBUG, oss);
        }
        // accumulated usage
        // int64_t sum_cpu[n_periods];
        // int64_t sum_mem[n_periods];
        // TODO(svallero): fix codestyle
        double sum_cpu[4];
        double sum_mem[4];
        // init values
        for (int k = 0; k < n_periods; k++) {
            sum_cpu[k] = 0.;
            sum_mem[k] = 0.;
        }
        // loop over accounting entries for user
        for (list<AcctObject*>::iterator j = acct_list.begin();
                                         j != acct_list.end(); ++j) {
            float cpu = (*j)->get_cpu();
            int memory = (*j)->get_memory();

            // loop over periods
            for (int k = 0; k < n_periods; k++) {
                // time always integrated from start_time
                // more convenient for plotting
                int64_t stop = time_stop - (k * period);
                int64_t stop_entry = (*j)->get_stop();
                int64_t start_entry = (*j)->get_start();
                // VM can still be running at "timestamp"
                if (!stop_entry || stop_entry > stop) stop_entry = stop;
                // VM might have started before "time_start"
                if (start_entry < time_start) start_entry = time_start;
                sum_cpu[k] += cpu * (stop_entry - start_entry);
                sum_mem[k] += memory * (stop_entry - start_entry);
            }  // end loop periods
        }  // end loop acct entries

        // set accounting entries in User object
        (*i).flush_usage();
         ostringstream output;
         output << "***** UID " << (*i).userID << "*****"<<endl;
        for (int k = 0; k < n_periods; k++) {
            struct Usage cpu_usage((int64_t)(sum_cpu[k]),
                                             time_start, time_stop);
            struct Usage mem_usage((int64_t)(sum_mem[k]),
                                             time_start, time_stop);
            (*i).set_cpu_usage(k, cpu_usage);
            (*i).set_memory_usage(k, mem_usage);
             output << "Start: " << time_start << " Stop: " << time_stop
             << " CPU usage: " << sum_cpu[k] << endl;
        }

        // const map<int, struct Usage> cpu_usage = (*i).get_cpu_usage();
        // map<int, struct Usage>::const_iterator usage_it;
        // for (usage_it = cpu_usage.begin(); usage_it != cpu_usage.end();
        //                                                     usage_it++) {
        //    struct Usage cpu = static_cast<struct Usage>(usage_it->second);
          // output << "Start: " << cpu.start_time << " Stop: " << cpu.stop_time
            // << " CPU usage: " << cpu.usage << endl;
        // }
        FassLog::log("AcctPool", Log::DEBUG, output);
    }  // end loop on users

    return 0;
}

void AcctPool::flush() {
    map<int, list<AcctObject*> >::iterator it;
    for (it = objects.begin(); it != objects.end(); it++) {
        list<AcctObject*>::iterator j;
        for (j = (it->second).begin(); j != (it->second).end(); ++j) {
            delete (*j);
        }
        (it->second).clear();
    }

    objects.clear();
}

void AcctPool::make_user_object(int uid, vector<xmlNodePtr> nodes) {
        // create a list of accounting objects per user
        list<AcctObject*> list_acct_user;

        for (unsigned int i = 0 ; i < nodes.size(); i++) {
            if ( nodes[i]== 0 || nodes[i]->children == 0 ||
                                             nodes[i]->children->next == 0 ) {
                FassLog::log("AcctPool", Log::ERROR,
                "XML Node does not represent a valid accounting entry");
                return;
            }
            AcctObject *acct = new AcctObject(nodes[i]);
            list_acct_user.push_back(acct);
            // delete acct;
        }


        objects.insert(pair<int, list<AcctObject*> >(uid, list_acct_user));
        // list_acct_user.clear();
        ostringstream   oss;
        oss << "Creating accounting object for user: " << uid;
        FassLog::log("AcctPool", Log::DDEBUG, oss);

        return;
}

int AcctPool::set_up(vector<int> const &uids) {
        int rc = 0;
        ostringstream   oss;

        // clean the pool to get updated data from OpenNebula
        flush();

        // load the complete accounnting pool from OpenNebula
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
        // xmlInitParser();
        if ( xml != 0 ) {
            xmlFreeDoc(xml);
        }

        if ( ctx != 0 ) {
            xmlXPathFreeContext(ctx);
        }

        // FassLog::log("SARA", Log::DEBUG, acctlist);
        xml_parse(acctlist);

        // loop over known users
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

            oss << "I got " << n_nodes << " nodes for user " << uid;
            FassLog::log("AcctPool", Log::DEBUG, oss);

            // creates a list of accounting entries per user
            // and add it to the map
            if (n_nodes) {
                make_user_object(uid, u_nodes);

                free_nodes(u_nodes);
            }
        }

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
