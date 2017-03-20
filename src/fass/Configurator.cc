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

// #include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include "FassLog.h"

void Configurator::print_loaded_options() {
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

Configurator::allowed_types Configurator::get_option_type(boost::any value) {
        const type_info& type = value.type();
        if (type == typeid (string)) return is_string;
        else if (type == typeid (int)) return is_int;
        else if (type == typeid (double)) return is_double;
        else if (type == typeid (bool)) return is_bool;
        else
          return is_unkn;
}

