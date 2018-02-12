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

#include "Fass.h"

// #include <stdlib.h>
// #include <libxml/parser.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

// #include "Log.h"
// #include "RPCManager.h"
// #include "PriorityManager.h"
#include "FassLog.h"
#include "Configurator.h"
#include "ObjectXML.h"
#include "FassDb.h"

void Fass::start(bool bootstrap_only) {
    bool             rc;
    // int             fd;
    sigset_t        mask;
    int             signal;
    char            hn[80];

    /// returns the null-terminated hostname in the character
    /// array hn, which has a length of 79 bytes.
    /// is hostname used somewhere?
    if ( gethostname(hn, 79) != 0 ) {
        throw runtime_error("Error getting hostname");
    }

    hostname = hn;

    /** Configuration system */
    fass_configuration = new FassConfigurator(etc_location, var_location);

    rc = fass_configuration->load_configuration();

    if ( !rc ) {
        throw runtime_error("Could not load Fass configuration file.");
    }

    /** Initial shares system */
    // Read initial shares from separate file
    initial_shares = new SharesConfigurator(etc_location);

    rc = initial_shares->load_shares();

    if ( !rc ) {
        throw runtime_error("Could not load initial shares file.");
    }

    /** Log system */

    ostringstream os;

    try {
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

        FassLog::log("FASS", Log::INFO, os);
        fass_configuration->print_loaded_options();

        os.clear();
        os.str("");
        os << "\n";
        os << "----------------------------------------\n";
        os << "     Initial Shares File      \n";
        os << "----------------------------------------\n";
        os << initial_shares->get_conf_fname()  << " \n";
        os << "----------------------------------------";

        FassLog::log("FASS", Log::INFO, os);
        initial_shares->print_shares();
    } catch(runtime_error&) {
        throw;
    }

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

    // TODO(svallero): should be done with a switch and enum, not critical
    if ( dbtype == "influxdb" ) {
        database = new InfluxDb(dbendpoint, dbport, dbname);
    } else {
       FassLog::log("FASS", Log::ERROR, "Unknown database type!");
       throw;
    }


    /** Managers */
    // some config variables are used both by the RPCManager
    // and the XMLRPCClient (called by the PM)
    // we use the same valuse for both server/client,
    // these should be consistent with the OpenNebula ones
    int64_t  message_size;
    int  timeout;
    int  manager_timer;  // used also by Terminator
    // OpenNebula xml-rpc endpoint
    string one_endpoint;
    string one_port;
    fass_configuration->get_single_option("fass", "one_endpoint", one_endpoint);
    fass_configuration->get_single_option("fass", "one_port", one_port);
    one_endpoint.append(":");
    one_endpoint.append(one_port);
    one_endpoint.append("/RPC2");
    // FassLog::log("FASS", Log::DEBUG, one_endpoint);
    // OpenNebula authentication
    string one_secret;
    fass_configuration->get_single_option("fass", "one_secret", one_secret);
    // xml-rpc config
    fass_configuration->get_single_option("rpcm", "message_size", message_size);
    fass_configuration->get_single_option("rpcm", "timeout", timeout);
    fass_configuration->get_single_option
         ("pm", "manager_timer", manager_timer);

    // Wait for ONE to become available
    while (1) {
        try {
            xmlrpc_c::value result;
            vector<xmlrpc_c::value> values;
            xmlrpc_c::paramList plist;
            // temporary client
            XMLRPCClient::initialize(one_secret, one_endpoint, message_size,
                                                                   timeout);

            XMLRPCClient *client = XMLRPCClient::client();

            client->call("one.system.config", plist, &result);

            values = xmlrpc_c::value_array(result).vectorValueValue();

            bool   success = xmlrpc_c::value_boolean(values[0]);
            string message = xmlrpc_c::value_string(values[1]);

            if ( !success ) {
                ostringstream oss;
                oss << "Cannot contact oned, will retry... Error: " << message;
                FassLog::log("FASS", Log::ERROR, oss);
            } else {
                // take manager timer from ONE and compare it to PM timer
                // ObjectXML * conf = new ObjectXML();
                // conf->cleanup();
                // conf->xml_parse(message);
                // int one_timer;
                // conf->xpath(one_timer, "/TEMPLATE/MANAGER_TIMER", 0);
                // if (manager_timer > one_timer) {
                //     manager_timer = one_timer - 1;
                //     ostringstream oss;
                //     oss
                //      << "PM timer is shorter than ONE timer, setting it to: "
                //                                             << manager_timer;
                //     FassLog::log("FASS", Log::WARNING, oss);
                // }
                // delete conf;
                break;
            }
        } catch (exception const& e) {
            ostringstream oss;
            oss << "Cannot contact oned, will retry... Error: " << e.what();
            FassLog::log("FASS", Log::ERROR, oss);
        }
        sleep(2);
    }

    FassLog::log("FASS", Log::INFO, "ONE successfully contacted.");
    FassLog::log("FASS", Log::INFO,
          "Make sure than the PM timer is not longer than ONE SCHED_INTERVAL!");

    /** Block all signals before creating any thread */

    sigfillset(&mask);

    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /** Initialize the XML library */
    xmlInitParser();

    /// ---- Priority Manager ----
    vector<string> shares;
    try {
        // int machines_limit;
        int period;
        int n_periods;
        int plugin_debug;
        string start_time;

        // fass_configuration->get_single_option
        //      ("pm", "max_vm", machines_limit);
        fass_configuration->get_single_option
             ("pm", "start_time", start_time);
        fass_configuration->get_single_option
             ("pm", "period", period);
        fass_configuration->get_single_option
             ("pm", "n_periods", n_periods);
        fass_configuration->get_single_option
             ("pm", "plugin_debug", plugin_debug);


        // Get the initial shares vector
        shares = initial_shares->get_shares();

        // for(vector<string>::const_iterator i = shares.begin();
        //                                   i != shares.end(); ++i) {
        //   FassLog::log("CICCIA", Log::INFO, * i);
        // }

        pm = new PriorityManager(one_endpoint, one_secret, message_size,
                   timeout, shares, manager_timer, start_time, database,
                                       period, n_periods, plugin_debug);
        }

    catch (bad_alloc&) {
        FassLog::log("FASS", Log::ERROR, "Error creating Priority Manager");
        throw;
    }

    /// ---- Start the Priority Manager ----

    rc = pm->start();

    if ( !rc ) {
       throw runtime_error("Could not start the Priority Manager");
    }

    /// ---- Terminator ----
    try {
        // int64_t ttl;
        int64_t max_wait;
        string action;
        fass_configuration->get_single_option
             ("terminator", "manager_timer", manager_timer);
        // fass_configuration->get_single_option
        //      ("terminator", "ttl", ttl);
        fass_configuration->get_single_option
             ("terminator", "max_wait", max_wait);
        fass_configuration->get_single_option
             ("terminator", "action", action);

        // tm = new Terminator(one_endpoint, one_secret, message_size,
        //                        timeout, manager_timer, shares, ttl, max_wait, action);
        tm = new Terminator(one_endpoint, one_secret, message_size,
                            timeout, manager_timer, shares, max_wait, action);
        }

    catch (bad_alloc&) {
        FassLog::log("FASS", Log::ERROR, "Error creating Terminator");
        throw;
    }

    /// ---- Start Terminator ----

     rc = tm->start();

    if ( !rc ) {
       throw runtime_error("Could not start Terminator");
    }

    /// ---- Request Manager ----
    try {
        // int  rm_port = 0;
        string rm_port = "";
        int  max_conn;
        int  max_conn_backlog;
        int  keepalive_timeout;
        int  keepalive_max_conn;
        bool rpc_log;
        string log_call_format;
        string rpc_filename = "";
        string rm_listen_address;  // = "0.0.0.0";

        fass_configuration->get_single_option
             ("rpcm", "listen_port", rm_port);
        fass_configuration->get_single_option
             ("rpcm", "listen_address", rm_listen_address);
        fass_configuration->get_single_option
             ("rpcm", "max_conn", max_conn);
        fass_configuration->get_single_option
             ("rpcm", "max_conn_backlog", max_conn_backlog);
        fass_configuration->get_single_option
             ("rpcm", "keepalive_timeout", keepalive_timeout);
        fass_configuration->get_single_option
             ("rpcm", "keepalive_max_conn", keepalive_max_conn);
        fass_configuration->get_single_option
             ("rpcm", "rpc_log", rpc_log);
        fass_configuration->get_single_option
             ("rpcm", "log_call_format", log_call_format);

        if (rpc_log) {
            rpc_filename = log_location + "fass_xmlrpc.log";
        }

        rpcm = new RPCManager(one_endpoint, rm_port, max_conn, max_conn_backlog,
            keepalive_timeout, keepalive_max_conn, timeout, rpc_filename,
            log_call_format, rm_listen_address, message_size);
    }

    catch (bad_alloc&) {
        FassLog::log("FASS", Log::ERROR, "Error creating RPC Manager");
        throw;
    }


    /// ---- Start the Request Manager ----

    rc = rpcm->start();

    if ( !rc ) {
       throw runtime_error("Could not start the RPC Manager");
    }



    /** Wait for a SIGTERM or SIGINT signal */

    sigemptyset(&mask);

    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    sigwait(&mask, &signal);

    /** Stop the managers and free resources */

    pm->finalize();
    tm->finalize();
    rpcm->finalize();

    // sleep to wait drivers???

    pthread_join(rpcm->get_thread_id(), 0);
    pthread_join(pm->get_thread_id(), 0);
    pthread_join(tm->get_thread_id(), 0);

    // XML Library
    xmlCleanupParser();

    FassLog::log("FASS", Log::INFO, "All modules finalized, exiting.\n");

    return;

// error_mad:
// Log::log("FASS", Log::ERROR, "Could not load driver");
// throw runtime_error("Could not load a Fass driver");
}

Log::MessageType Fass::get_debug_level() const {
    Log::MessageType clevel = Log::ERROR;

    int log_level_int;

    fass_configuration->get_single_option("fass", "log_level", log_level_int);

    if ( log_level_int != 0 ) {
        if ( Log::ERROR <= log_level_int && log_level_int <= Log::DDDEBUG ) {
            clevel = static_cast<Log::MessageType>(log_level_int);
        }
    }

    return clevel;
}
