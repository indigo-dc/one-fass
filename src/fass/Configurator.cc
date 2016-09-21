/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#include "Configurator.h"
#include "FassLog.h"

//#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

//#include <unistd.h>
//#include <sys/stat.h>


using namespace std;

/* -------------------------------------------------------------------------- */

bool Configurator::load_configuration(){

    // Cannot use logsystem yet...
    cout << "Loading configuration..." << endl;
    // Declare the supported options.
    po::options_description desc("Allowed options");

    desc.add_options()
    		("help", "produce help message")
                // General 
    		("fass.manager_timer", po::value<int>()->default_value(60), "managers period (s)")
     		("fass.one_port", po::value<int>()->default_value(2633), "OpenNebula listen port")
    		("fass.one_endpoint", po::value<string>()->default_value("localhost"), "OpenNebula listen endpoint")
    		("fass.log_level", po::value<int>()->default_value(3), "log level common to all managers")
		// RPC manager
    		("rpcm.listen_port", po::value<int>()->default_value(2634), "listen port")
    		("rpcm.listen_address", po::value<string>()->default_value("127.0.0.1"), "listen address")
    		("rpcm.max_conn", po::value<int>()->default_value(15), "max connections")
    		("rpcm.max_conn_backlog", po::value<int>()->default_value(15), "max connections backlog")
    		("rpcm.keepalive_timeout", po::value<int>()->default_value(15), "keepalive timeout")
    		("rpcm.keepalive_max_conn", po::value<int>()->default_value(30), "keepalive max connections")
    		("rpcm.timeout", po::value<int>()->default_value(15), "timeout")
    		("rpcm.rpc_log", po::value<bool>()->default_value(true), "separate rpc log")
    		("rpcm.message_size", po::value<int>()->default_value(1073741824), "message size")
    		("rpcm.log_call_format", po::value<string>()->default_value("Req:%i UID:%u %m invoked %l"), "log call format")
    		;

    // Read the configuration file
    ifstream settings_file(conf_file.c_str());

    // Clear the map
    vm = po::variables_map();

    try{
        po::store(po::parse_config_file(settings_file , desc), vm);
    } catch(const exception& re){
        cerr << "Error: " << re.what() << endl;
        return false;
    }

    po::notify(vm);     
    
    return true;
}

/* -------------------------------------------------------------------------- */

void Configurator::print_loaded_options(){
        ostringstream os;
        os <<"Reading configuration..." << endl;
        for (po::variables_map::iterator it = vm.begin(); it != vm.end(); it++) {

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
                switch (tp){
                        case is_string: os << boost::any_cast<string>(vm[opt_name].value());
                        break;
                        case is_int: os << boost::any_cast<int>(vm[opt_name].value());
                        break;
                        case is_double: os << boost::any_cast<double>(vm[opt_name].value());
                        break;
                        case is_bool: os << boost::any_cast<bool>(vm[opt_name].value());
                        break;
                        case is_unkn: os << "UnknownType";
                        break;
                }
                os << endl;
                                                                               
        }                                                                      
        FassLog::log("CONF",Log::INFO,os); 
}

/* -------------------------------------------------------------------------- */

Configurator::allowed_types Configurator::get_option_type(boost::any value){

        const type_info& type = value.type();
        if (type == typeid (string)) return is_string;
        else if (type == typeid (int)) return is_int;
        else if (type == typeid (double)) return is_double;
        else if (type == typeid (bool)) return is_bool;
        else return is_unkn;

}

/* -------------------------------------------------------------------------- */

const char * FassConfigurator::conf_name="fassd.conf";

