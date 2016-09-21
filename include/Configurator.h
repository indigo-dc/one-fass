/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include "FassLog.h"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;
namespace po = boost::program_options;
 
/* This class provides the basic abstraction for Fass configuration files */

class Configurator
{
public:
    Configurator(const string& etc_location, const char * _conf_name)
    {
        conf_file = etc_location + _conf_name;
    }

    virtual ~Configurator(){};
   
    /* Parse and loads the configuration */
    bool load_configuration();

    /* Get the configuration filename */
    string get_conf_fname(){ return conf_file; };

    /* Prints loaded configuration*/
    void print_loaded_options(); 
 
protected:
    /* Name for the configuration file, fassd.conf */
    string conf_file;

    /* Map containing configuration variables */
    po::variables_map   vm;

    /* Allowed option types */
    enum allowed_types {is_string, is_int, is_double, is_bool, is_unkn};

    /* Gets the option value type */
    allowed_types get_option_type(boost::any value);

    /* Gets a single configuration value from specified section*/
    /* REMEMBER: methods with templates must be 
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

    /* Gets the single option value */ 
    template<class T> 
    bool get_single_option(const string section, const string name, T& value) const {

       return get_option(section, name, value);	
       
   }

private:
    static const char * conf_name;
    /* Path for the var directory, for defaults */
    string var_location;
};

// TODO: poi ci vanno le classi per configurare gli algoritmi, le quote etc...

#endif /*CONFIGURATOR_H_*/
