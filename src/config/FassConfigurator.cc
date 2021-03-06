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
#include "FassLog.h"
#include "Configurator.h"

/* -------------------------------------------------------------------------- */

bool FassConfigurator::load_configuration() {
    /// Cannot use logsystem yet...
    cout << "Loading configuration..." << endl;
    /// Declare the supported options.
    po::options_description desc("Allowed options");

    desc.add_options()
      ("help", "produce help message")
      /// General
      ("fass.one_port", po::value<string>()->default_value("2633"),
       "OpenNebula listen port")
      ("fass.one_endpoint", po::value<string>()
                            ->default_value("http://localhost"),
       "OpenNebula listen endpoint")
      ("fass.one_secret", po::value<string>()
                          ->default_value("oneadmin:opennebula"),
       "OpenNebula authentication")
      ("fass.log_level", po::value<int>()->default_value(3),
       "log level common to all managers")
       /// RPC manager
      ("rpcm.listen_port", po::value<string>()->default_value("2634"),
       "listen port")
      ("rpcm.listen_address", po::value<string>()->default_value("127.0.0.1"),
       "listen address")
      ("rpcm.timeout", po::value<int>()->default_value(15),
       "timeout")
      ("rpcm.message_size", po::value<int64_t>()->default_value(1073741824),
       "message size")
      ("rpcm.max_conn", po::value<int>()->default_value(15),
       "max connections")
      ("rpcm.max_conn_backlog", po::value<int>()->default_value(15),
       "max connections backlog")
      ("rpcm.keepalive_timeout", po::value<int>()->default_value(15),
       "keepalive timeout")
      ("rpcm.keepalive_max_conn", po::value<int>()->default_value(30),
       "keepalive max connections")
      ("rpcm.rpc_log", po::value<bool>()->default_value(true),
       "separate rpc log")
      ("rpcm.log_call_format", po::value<string>()
                            ->default_value("Req:%i UID:%u %m invoked %l"),
       "log call format")
      // Database
      ("database.type", po::value<string>()->default_value("influxdb"),
       "DB back-end")
      ("database.endpoint", po::value<string>()->default_value("localhost"),
       "DB endpoint")
      ("database.port", po::value<int>()->default_value(8086),
       "DB listen port")
      ("database.name", po::value<string>()->default_value("fassdb"),
       "DB name")
      // Priority manager
      ("pm.manager_timer", po::value<int>()->default_value(60),
       "manager period (s)")
      ("pm.start_time", po::value<string>()->default_value("19/04/1981"),
       "to start collecting accounting info (day/month/year)")
      ("pm.max_vm", po::value<int>()->default_value(5000),
       "Maximum number of Virtual Machines scheduled")
      ("pm.period", po::value<int>()->default_value(180),
       "for historical usage, it multiplies manager_timer")
      ("pm.n_periods", po::value<int>()->default_value(3),
       "number of periods used in the algorithm")
      ("pm.plugin_debug", po::value<int>()->default_value(0),
       "Use dummy algo if set to 0, use simple fairshare algo otherwise")
      // Terminator
      ("terminator.manager_timer", po::value<int>()->default_value(60),
       "manager period (s)")
      // ("terminator.ttl", po::value<int64_t>()->default_value(1000),
      //  "VMs time to live")
      ("terminator.max_wait", po::value<int64_t>()->default_value(3600),
       "VMs max waiting time")
      ("terminator.action", po::value<string>()->default_value("kill"),
       "action to perform after ttl");


    /// Read the configuration file
    ifstream settings_file(conf_file.c_str());

    // Clear the map
    vm = po::variables_map();
    // temporary streamer map
    po::variables_map vm_tmp = po::variables_map();

    try {
        po::store(po::parse_config_file(settings_file , desc), vm_tmp);
    } catch(const exception& re) {
        cerr << "Error: " << re.what() << endl;
        return false;
    }

    po::notify(vm_tmp);

    swap(vm_tmp, vm);
    settings_file.close();
    return true;
}

/* -------------------------------------------------------------------------- */

void FassConfigurator::print_loaded_options() {
        ostringstream os;
        os <<"Reading configuration..." << endl;
        for (po::variables_map::iterator it = vm.begin();
             it != vm.end(); it++) {
                string opt_name = it->first;
                os << "> " << opt_name;

                if (((boost::any)it->second.value()).empty()) {
                        os << "(empty)";
                }
                if (vm[opt_name].defaulted() || it->second.defaulted()) {
                        os << "(default)";
                }
                os << "=";

                allowed_types tp = get_option_type(it->second.value());
                switch (tp) {
                        case is_string: os
                                        << boost::any_cast<string>
                                           (vm[opt_name].value());
                        break;
                        case is_long_int: os
                                        << boost::any_cast<int64_t>
                                           (vm[opt_name].value());
                        break;
                        case is_int: os
                                        << boost::any_cast<int>
                                           (vm[opt_name].value());
                        break;
                        case is_double: os
                                        << boost::any_cast<double>
                                           (vm[opt_name].value());
                        break;
                        case is_bool: os
                                        << boost::any_cast<bool>
                                           (vm[opt_name].value());
                        break;
                        case is_unkn: os << "UnknownType";
                        break;
                }
                os << endl;
        }

        FassLog::log("CONF", Log::INFO, os);
}

/* -------------------------------------------------------------------------- */

const char * FassConfigurator::conf_name = "fassd.conf";
