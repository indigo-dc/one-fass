/**
 * InitShares.h
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#ifndef INITSHARES_H_
#define INITSHARES_H_

#include "FassLog.h"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;
namespace po = boost::program_options;
 
/** This class provides the basic abstraction for Fass configuration files */

class InitShares 
{
public:
    InitShares(const string& etc_location, const char * _shares_name)
    {
        initset_file = etc_location + _shares_name;
    }

    virtual ~InitShares(){};
   
    /** Parse and loads the configuration */
    bool load_shares();

    /** Get the configuration filename */
    string get_shares_fname(){ return initset_file; };

 
protected:
    /** Name for the configuration file, fassd.conf */
    string initset_file;

    /** Map containing configuration variables */
    po::variables_map   vm;

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

private:

    // dummy

};

// -----------------------------------------------------------------------------

class FassInitShares : public InitShares
{
public:

    FassInitShares(const string& etc_location, const string& _var_location):
        InitShares(etc_location, initset_name), var_location(_var_location)
        {};

    ~FassInitShares(){};

    /** Gets the single option value */ 
    template<class T> 
    bool get_single_option(const string section, const string name, T& value) const {

       return get_option(section, name, value);	
       
   }

private:
    static const char * initset_name;
    /** Path for the var directory, for defaults */
    string var_location;
};

/// TODO: classes to configure algorithms, quotas etc... 

#endif /*CONFIGURATOR_H_*/
