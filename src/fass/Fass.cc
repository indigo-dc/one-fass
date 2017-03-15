/**
 * Fass.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */
#include "Fass.h"
#include "FassLog.h"
#include "Configurator.h"
#include "FassDb.h"
//#include "Log.h"
//#include "RPCManager.h"
//#include "PriorityManager.h"


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

    /// returns the null-terminated hostname in the character
    /// array hn, which has a length of 79 bytes.
    /// is hostname used somewhere?
    if ( gethostname(hn,79) != 0 )
    {
        throw runtime_error("Error getting hostname");
    }

    hostname = hn;

    /** Configuration system */
    fass_configuration = new FassConfigurator(etc_location, var_location);

    rc = fass_configuration->load_configuration();

    if ( !rc )
    {
        throw runtime_error("Could not load Fass configuration file.");
    }

    /** Log system */

    ostringstream os;

    try{
        Log::MessageType   clevel;

        clevel     = get_debug_level();

        /// Initializing FASS daemon log system
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



    /** Initialize the XML library */
    // Try to avoid it
    //    xmlInitParser();


    /** Database */

    // instance of specific DB (can be changed)
    string dbtype;
    string dbendpoint;
    string dbname;
    int dbport;
    fass_configuration->get_single_option("database", "type", dbtype);
    fass_configuration->get_single_option("database", "endpoint", dbendpoint);
    fass_configuration->get_single_option("database", "port", dbport);
    fass_configuration->get_single_option("database", "name", dbname);
    if ( dbtype == "influxdb" ){ // TODO: should be done with a switch and enum, not critical
       database = new InfluxDb(dbendpoint, dbport, dbname);
    } else {
       FassLog::log("FASS", Log::ERROR, "Unknown database type!");
       throw; 
    }
   
    // FOR VALE: database should be passed as argument to the Priority Manager

    /** Block all signals before creating any thread */

    sigfillset(&mask);

    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /** Managers */
    // some config variables are used both by the RPCManager and the XMLRPCClient (called by the PM)
    // we use the same valuse for both server/client, these should be consistent with the OpenNebula ones
    int  message_size;
    int  timeout;
    // OpenNebula xml-rpc endpoint
    string one_endpoint;
    string one_port;
    fass_configuration->get_single_option("fass", "one_endpoint", one_endpoint);
    fass_configuration->get_single_option("fass", "one_port", one_port);
    one_endpoint.append(":");
    one_endpoint.append(one_port);
    one_endpoint.append("/RPC2");
    //FassLog::log("FASS", Log::DEBUG, one_endpoint);
    // OpenNebula authentication 
    string one_secret;
    fass_configuration->get_single_option("fass", "one_secret", one_secret);
    // xml-rpc config
    fass_configuration->get_single_option("rpcm", "message_size", message_size);
    fass_configuration->get_single_option("rpcm", "timeout", timeout);

   /// ---- Priority Manager ----
   try
   {
    int  manager_timer;
    int machines_limit;

    fass_configuration->get_single_option("pm", "max_vm", machines_limit);
    fass_configuration->get_single_option("pm", "manager_timer", manager_timer);
 
    pm = new PriorityManager(one_endpoint, one_secret, message_size, timeout, manager_timer, machines_limit);
    }

    catch (bad_alloc&)
    {
        FassLog::log("FASS", Log::ERROR, "Error creating Priority Manager");
        throw;
    }

    /// ---- Start the Priority Manager ----

    rc = pm->start();

    if ( !rc )
    {
       throw runtime_error("Could not start the Priority Manager");
    }


    /// ---- Request Manager ----
    try
    {

        //int  rm_port = 0;
        string rm_port = "";
	int  max_conn;
        int  max_conn_backlog;
        int  keepalive_timeout;
        int  keepalive_max_conn;
        bool rpc_log;
        string log_call_format;
        string rpc_filename = "";
        string rm_listen_address; //= "0.0.0.0";

        fass_configuration->get_single_option("rpcm", "listen_port", rm_port);
        fass_configuration->get_single_option("rpcm", "listen_address", rm_listen_address);
        fass_configuration->get_single_option("rpcm", "max_conn", max_conn);
        fass_configuration->get_single_option("rpcm", "max_conn_backlog", max_conn_backlog);
        fass_configuration->get_single_option("rpcm", "keepalive_timeout", keepalive_timeout);
        fass_configuration->get_single_option("rpcm", "keepalive_max_conn", keepalive_max_conn);
        fass_configuration->get_single_option("rpcm", "rpc_log", rpc_log);
        fass_configuration->get_single_option("rpcm", "log_call_format", log_call_format);

        if (rpc_log)
        {
            rpc_filename = log_location + "fass_xmlrpc.log";
        }

        rpcm = new RPCManager(one_endpoint,rm_port, max_conn, max_conn_backlog,
            keepalive_timeout, keepalive_max_conn, timeout, rpc_filename,
            log_call_format, rm_listen_address, message_size);

    }

    catch (bad_alloc&)
    {
        FassLog::log("FASS", Log::ERROR, "Error creating RPC Manager");
        throw;
    }


    /// ---- Start the Request Manager ----

    rc = rpcm->start();

    if ( !rc )
    {
       throw runtime_error("Could not start the RPC Manager");
    }



    /** Wait for a SIGTERM or SIGINT signal */

    sigemptyset(&mask);

    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    sigwait(&mask, &signal);

    /** Stop the managers and free resources */

    pm->finalize();  

    //sleep to wait drivers???

    pthread_join(rpcm->get_thread_id(),0);
    pthread_join(pm->get_thread_id(),0);

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
    Log::MessageType clevel = Log::ERROR;
    
    int              log_level_int ;

    fass_configuration->get_single_option("fass", "log_level", log_level_int);  

    if ( log_level_int != 0 )
    {

        if ( Log::ERROR <= log_level_int && log_level_int <= Log::DDDEBUG )
        {
            clevel = static_cast<Log::MessageType>(log_level_int);
        }
    }

    return clevel;
}
