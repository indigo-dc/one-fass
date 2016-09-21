/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#include "Fass.h"
#include "FassLog.h"
#include "Configurator.h"
//#include "Log.h"
//#include "RPCManager.h"

#include <fstream>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
//#include <stdlib.h>
//#include <libxml/parser.h>

//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <pthread.h>
#include <iostream>

using namespace std;


void Fass::start(bool bootstrap_only)
{

    bool             rc;
    //int             fd;
    sigset_t        mask;
    int             signal;
    char            hn[80];

    // SV: a cosa serve?
    if ( gethostname(hn,79) != 0 )
    {
        throw runtime_error("Error getting hostname");
    }

    hostname = hn;

    /* Configuration system */
    fass_configuration = new FassConfigurator(etc_location, var_location);

    rc = fass_configuration->load_configuration();

    if ( !rc )
    {
        throw runtime_error("Could not load Fass configuration file.");
    }

    /* Log system */

    ostringstream os;

    try{
        Log::MessageType   clevel;

        clevel     = get_debug_level();

        // Initializing FASS daemon log system
        string log_fname;
        log_fname = log_location + "fass.log";
        FassLog::init_log_system(clevel,
                           log_fname.c_str(),
                           ios_base::trunc,
                           "fassd");
        

        os << "Starting " << version() << endl;
        os << "----------------------------------------\n";
        os << "     Fass Configuration File      \n";
        os << "----------------------------------------\n";
        os << fass_configuration->get_conf_fname()  << " \n";
        os << "----------------------------------------";
       
        FassLog::log("FASS",Log::INFO,os);
        fass_configuration->print_loaded_options(); 
    } catch(runtime_error&){

        throw;
    }

    /* Initialize the XML library */
    // spero di riuescire ad evitarlo...
    //    xmlInitParser();


    /* Database */

    // TODO: qui bisogna capire come procedere...

    /* Block all signals before creating any thread */

    sigfillset(&mask);

    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /* Get manager timer from config */
    int  manager_timer;
    fass_configuration->get_single_option("fass", "manager_timer",manager_timer);

    /* Managers */

    // ---- Request Manager ----
    try
    {
        int  rm_port = 0;
        int  max_conn;
        int  max_conn_backlog;
        int  keepalive_timeout;
        int  keepalive_max_conn;
        int  timeout;
        bool rpc_log;
        string log_call_format;
        string rpc_filename = "";
        int  message_size;
        string rm_listen_address; //= "0.0.0.0";

        fass_configuration->get_single_option("rpcm", "listen_port", rm_port);
        fass_configuration->get_single_option("rpcm", "listen_address", rm_listen_address);
        fass_configuration->get_single_option("rpcm", "max_conn", max_conn);
        fass_configuration->get_single_option("rpcm", "max_conn_backlog", max_conn_backlog);
        fass_configuration->get_single_option("rpcm", "keepalive_timeout", keepalive_timeout);
        fass_configuration->get_single_option("rpcm", "keepalive_max_conn", keepalive_max_conn);
        fass_configuration->get_single_option("rpcm", "timeout", timeout);
        fass_configuration->get_single_option("rpcm", "rpc_log", rpc_log);
        fass_configuration->get_single_option("rpcm", "message_size", message_size);
        fass_configuration->get_single_option("rpcm", "log_call_format", log_call_format);

        if (rpc_log)
        {
            rpc_filename = log_location + "fass_xmlrpc.log";
        }

        rpcm = new RPCManager(rm_port, max_conn, max_conn_backlog,
            keepalive_timeout, keepalive_max_conn, timeout, rpc_filename,
            log_call_format, rm_listen_address, message_size);

    }

    catch (bad_alloc&)
    {
        FassLog::log("FASS", Log::ERROR, "Error creating RPC Manager");
        throw;
    }


    // ---- Start the Request Manager ----

    rc = rpcm->start();

    if ( !rc )
    {
       throw runtime_error("Could not start the RPC Manager");
    }

    /* Wait for a SIGTERM or SIGINT signal */

    sigemptyset(&mask);

    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    sigwait(&mask, &signal);

    /* Stop the managers and free resources */

    //rpcm->finalize(); devi usare l'action manager

    //sleep to wait drivers???

    pthread_join(rpcm->get_thread_id(),0);

    //XML Library
//    xmlCleanupParser();

    FassLog::log("FASS", Log::INFO, "All modules finalized, exiting.\n");

    return;

//error_mad:
//    Log::log("FASS", Log::ERROR, "Could not load driver");
//    throw runtime_error("Could not load a Fass driver");
}

Log::MessageType Fass::get_debug_level() const
{
    //Log::MessageType clevel = Log::ERROR;
    Log::MessageType clevel = Log::DDDEBUG;
    
/* SV: da sistemare 
    int              log_level_int;

    const VectorAttribute * log = nebula_configuration->get("LOG");

    if ( log != 0 )
    {
        string value = log->vector_value("DEBUG_LEVEL");

        log_level_int = atoi(value.c_str());

        if ( Log::ERROR <= log_level_int && log_level_int <= Log::DDDEBUG )
        {
            clevel = static_cast<Log::MessageType>(log_level_int);
        }
    }
*/
    return clevel;
}
