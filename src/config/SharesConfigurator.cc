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

#include <fstream>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include "FassLog.h"
#include "Configurator.h"

/* -------------------------------------------------------------------------- */

bool SharesConfigurator::print_shares() {
    ostringstream os;
    os << "Reading initial shares..." << endl;
    os << "(username:uid:gid:share)" << endl;

    for (vector<string>::const_iterator i = shares.begin();
                                      i != shares.end(); ++i) {
       os << "> " << *i << endl;
    }

    FassLog::log("CONF", Log::INFO, os);
    return true;
}

/* -------------------------------------------------------------------------- */

bool SharesConfigurator::load_shares() {
    cout << "Loading shares..." << endl;

    ostringstream os;
    vector<string> properties(props, props + 3);

    boost::property_tree::ptree pt;
    boost::property_tree::ptree pt_tmp;

    // pthread_mutex_t mutex;
    // pthread_mutex_lock(&mutex);
    // int th;

    ifstream settings_file(conf_file.c_str());
    try {
       // Read the configuration file
       boost::property_tree::ini_parser::read_ini(settings_file, pt_tmp);
    } catch(const exception& re) {
       cerr << "Error: " << re.what() << endl;
       return false;
    }

    // pthread_mutex_unlock(&mutex);
    for (boost::property_tree::ptree::const_iterator it = pt_tmp.begin();
                                                    it != pt_tmp.end(); ++it) {
       string user = it->first;
       string value(user);
       value.append(":");
       for (vector<string>::const_iterator i = properties.begin();
                                         i != properties.end(); ++i) {
          try {
              string search(user);
              search = search.append(".");
              search = search.append(*i);
              value.append(pt_tmp.get<std::string>(search.c_str()));
              value.append(":");
          } catch(const exception& re) {
              os << "Error: " << re.what() << endl;
              FassLog::log("CONF", Log::INFO, os);
              return false;
          }
       }  // end loop on properties
       value.erase(value.size() - 1);
       shares.push_back(value);
    }  // end loop on users

    swap(pt_tmp, pt);
    // pt_tmp.clear();
    settings_file.close();
    return true;
}

/* -------------------------------------------------------------------------- */

const char * SharesConfigurator::conf_name = "shares.conf";

/* -------------------------------------------------------------------------- */

const char * SharesConfigurator::props[]={"uid", "gid", "share"};
