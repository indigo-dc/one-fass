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

#include "Configurator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "FassLog.h"

/* -------------------------------------------------------------------------- */

bool SharesConfigurator::load_shares() {
    /// Cannot use logsystem yet...
    cout << "Loading shares..." << endl;

    // Declare the supported options.
    // po::options_description desc("Allowed options");

    // desc.add_options()
    //  ("help", "produce help message")
      // General
      // ("fass.one_port", po::value<string>()->default_value("2633"),

    // Read the configuration file
    // ifstream settings_file(conf_file.c_str());

    // Clear the map
    // vm = po::variables_map();

    // try {
    //    po::store(po::parse_config_file(settings_file , desc), vm);
    // } catch(const exception& re) {
    //    cerr << "Error: " << re.what() << endl;
    //    return false;
    // }

    // po::notify(vm);

    return true;
}

/* -------------------------------------------------------------------------- */

const char * SharesConfigurator::conf_name = "shares.conf";
