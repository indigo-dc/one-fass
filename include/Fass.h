/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef FASS_H_
#define FASS_H_

#include "Configurator.h"
#include "Log.h"
#include "RPCManager.h"

#include <stdlib.h>
#include <string>
//#include <sstream>


using namespace std;

/* This is the main class for the Fass daemon fassd */

class Fass
{
public:

    static Fass& instance()
    {
        static Fass fassd;

        return fassd;
    };


/*
    // Manager getters 

    RPCManager * get_rpcm()
    {
        return rpcm;
    };

*/
    // Environment and Configuration 

    // Logging: to keep it simple we only foresee logging to a file
    // (no syslog)

    Log::MessageType get_debug_level() const;

    // REturns the value of FASS_LOCATION env variable. When this variable is
    // not defined the fass location is "/".

    const string& get_fass_location()
    {
        return fass_location;
    };

    // Returns the version of fassd
    static string version()
    {
        return "Fass 0.0";
    };


    // Start all the managers for Fass 
    void start(bool bootstrap_only=false);


    // Initialize the database 
    void bootstrap_db()
    {
        start(true);
    }

/*
    // Configuration attributes (read from fass.conf) 

    template<typename T> 
    void get_configuration_attribute(const string& name, T& value) const
    {
        fass_configuration->get(name, value);
    };
*/

private:

    // Constructors and = are private to only access the class through instance. 

    Fass()//:fass_configuration(0)
    {
        const char * nl = getenv("FASS_LOCATION");

        if (nl == 0) //Fass installed under root directory
        {
            fass_location = "/";

            etc_location     = "/etc/fass/";
            log_location     = "/var/log/fass/";
            var_location     = "/var/lib/fass/";
        }
        else
        {
            fass_location = nl;

            if ( fass_location.at(fass_location.size()-1) != '/' )
            {
                fass_location += "/";
            }

            etc_location     = fass_location + "etc/";
            log_location     = fass_location + "log/";
            var_location     = fass_location + "var/";
        }
    };


    ~Fass()
    {
        delete fass_configuration;
        delete rpcm;
        //delete db;
    };

    Fass& operator=(Fass const&){return *this;};

    // Environment variables 
    string  fass_location;
    string  etc_location;
    string  log_location;
    string  var_location;
    string  hostname;


    // Configuration 

    FassConfigurator * fass_configuration;



    //  Fass Managers 

    RPCManager *        rpcm;

/*
    // Implementation functions 
    // SV: cosa serve sta roba?

    friend void fass_signal_handler (int sig);

    // Helper functions 
    
    // Gets a Generic configuration attribute

    int get_conf_attribute(
        const string& key,
        const string& name,
        const VectorAttribute* &value) const;
*/
};

#endif /*FASS_H_*/
