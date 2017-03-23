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

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_
#include "FassLog.h"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;
namespace po = boost::program_options;
 
/** This class provides the basic abstraction for Fass configuration files */

class Configurator
{
public:
    Configurator(const string& etc_location, const char * _conf_name)
    {
        conf_file = etc_location + _conf_name;
    }

    virtual ~Configurator(){};
   

    /** Get the configuration filename */
    string get_conf_fname(){ return conf_file; };

 
protected:
    /** Name for the configuration file, fassd.conf */
    string conf_file;


    /** Allowed option types */
    enum allowed_types {is_string, is_int, is_double, is_bool, is_unkn};

    /** Gets the option value type */
    allowed_types get_option_type(boost::any value);

    /** Gets a single configuration value from specified section*/
    /** REMEMBER: methods with templates must be 
       declared and implemented IN THE SAME FILE! */

    template<typename T> 
    bool get_option(const string section, const string name, T& value) const {
        
    	string fullname = section + "." +  name;
 
        bool retval = true;

        try{
        	value =  boost::any_cast<T>(vm[fullname].value());
        } catch (exception& e){
		cout << e.what() << endl;
                retval = false; 
		}

    	return retval;
    } 

    /** Map containing configuration variables */
    po::variables_map   vm;

private:

    // dummy

};

// -----------------------------------------------------------------------------

class FassConfigurator : public Configurator
{
public:

    FassConfigurator(const string& etc_location, const string& _var_location):
        Configurator(etc_location, conf_name), var_location(_var_location)
        {};

    ~FassConfigurator(){};

    /** Parse and loads the configuration */
    bool load_configuration();

    /** Prints loaded configuration*/
    void print_loaded_options(); 

    /** Gets the single option value */ 
    template<class T> 
    bool get_single_option(const string section, const string name, T& value) const {

       return get_option(section, name, value);	
       
   }

private:
    static const char * conf_name;

    /** Path for the var directory, for defaults */
    string var_location;

};

// -----------------------------------------------------------------------------

class SharesConfigurator : public Configurator
{
public:

    SharesConfigurator(const string& etc_location):
        Configurator(etc_location, conf_name)
        {};


    ~SharesConfigurator(){};

    /** Parse and loads the configuration */
    bool load_shares();

    /** Prints loaded initila shares */
    bool print_shares(); 

    /** Returns a vector of strings in the format: */
    // "uid:gid:share"
    const vector<string>&  get_shares() const {
       return shares;
    };

private:
    static const char * conf_name;
    
    static const char * props[];

    /** To store initial shares */
    vector<string> shares;
};

/// TODO: classes to configure algorithms, quotas etc... 

#endif /*CONFIGURATOR_H_*/
