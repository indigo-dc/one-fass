/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */
#ifndef RPC_MANAGER_H_
#define RPC_MANAGER_H_

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

using namespace std;

//extern "C" void * rm_action_loop(void *arg);

extern "C" void * rm_xml_server_loop(void *arg);

class RPCManager
{
public:

    RPCManager(
            int _port,
            int _max_conn,
            int _max_conn_backlog,
            int _keepalive_timeout,
            int _keepalive_max_conn,
            int _timeout,
            const string _xml_log_file,
            const string call_log_format,
            const string _listen_address,
            int message_size);

    ~RPCManager(){};
 
    /* This functions starts the associated listener thread (XML server), and
       creates a new thread for the Request Manager. This thread will wait in
       an action loop till it receives ACTION_FINALIZE. */

    bool start();
   
    pthread_t get_thread_id() const
    {
        return rm_thread;
    };


private:

    friend void * rm_xml_server_loop(void *arg);
    //friend void * rm_action_loop(void *arg);    

    pthread_t               rm_thread;
    pthread_t               rm_xml_server_thread;


    int port;
    int socket_fd;
    int max_conn;
    int max_conn_backlog;
    int keepalive_timeout;
    int keepalive_max_conn;
    int timeout;
    
    string xml_log_file;
    string listen_address;
   
     
    bool setup_socket();
    void register_xml_methods();


    xmlrpc_c::registry RPCManagerRegistry;
   
    xmlrpc_c::serverAbyss *  AbyssServer;
};

#endif
