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

#ifndef RPC_MANAGER_H_
#define RPC_MANAGER_H_

#include "Manager.h"

#include <unistd.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

using namespace std;

//extern "C" void * rm_action_loop(void *arg);

extern "C" void * rm_xml_server_loop(void *arg);

class RPCManager: public Manager
{
public:

    RPCManager(
        const string& _one_endpoint,
        const string& _port,
//            int _port,
            int _max_conn,
            int _max_conn_backlog,
            int _keepalive_timeout,
            int _keepalive_max_conn,
            int _timeout,
            const string _xml_log_file,
            const string _call_log_format,
            const string _listen_address,
            int _message_size);

    ~RPCManager(){};
 
    /** This functions starts the associated listener thread (XML server), and
       creates a new thread for the Request Manager. This thread will wait in
       an action loop till it receives ACTION_FINALIZE. */

    int start();
   
    pthread_t get_thread_id() const
    {
        // return rm_thread;
        return rm_xml_server_thread;
    };


    void finalize() {
        lock();
        AbyssServer->terminate();
        pthread_cond_signal(&cond);
        pthread_cancel(rm_xml_server_thread);
        pthread_join(rm_xml_server_thread,0);
        // AbyssServer->terminate();
        delete AbyssServer;
        if ( socket_fd != -1 )
        {
            close(socket_fd);
        }
        unlock();
    };
private:
    
    /// Friends, thread functions require C-linkage

    friend void * rm_xml_server_loop(void *arg);
    //friend void * rm_action_loop(void *arg);    

    pthread_attr_t          pattr;   /// Thread attributes
    pthread_t               rm_thread; /// Thread ID for the RPCMananger
    pthread_t               rm_xml_server_thread; /// Thread ID for the XML server


    string one_endpoint;
    string port;
    //int port; /// Port number where connection is opened
    int socket_fd;
    int max_conn;
    int max_conn_backlog;
    int keepalive_timeout;
    int keepalive_max_conn;
    int timeout;
    
    string xml_log_file;
    string call_log_format;
    string listen_address;
    int message_size;
   
     
    bool setup_socket();
    int  setup_socket_new();
    void register_xml_methods();


    xmlrpc_c::registry RPCManagerRegistry;
   
    xmlrpc_c::serverAbyss *  AbyssServer;
};

#endif
