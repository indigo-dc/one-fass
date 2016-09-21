/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */
#include "Fass.h"
#include "FassLog.h"
#include "RPCManager.h"
#include "RequestSystem.h"
#include "RequestOneProxy.h"
#include "Request.h"

#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <sys/signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

RPCManager::RPCManager(
        int _port,
        int _max_conn,
        int _max_conn_backlog,
        int _keepalive_timeout,
        int _keepalive_max_conn,
        int _timeout,
        const string _xml_log_file,
        const string call_log_format,
        const string _listen_address,
        int message_size):
            port(_port),
            socket_fd(-1),
            max_conn(_max_conn),
            max_conn_backlog(_max_conn_backlog),
            keepalive_timeout(_keepalive_timeout),
            keepalive_max_conn(_keepalive_max_conn),
            timeout(_timeout),
            xml_log_file(_xml_log_file),
            listen_address(_listen_address)
{
    Request::set_call_log_format(call_log_format);

    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, message_size);

    // TODO: secondo me non ci serve
    //am.addListener(this);
};

extern "C" void * rm_xml_server_loop(void *arg)
{
    RPCManager *    rm;

    if ( arg == 0 )
    {
        return 0;
    }

    rm = static_cast<RPCManager *>(arg);
    
    // Set cancel state for the thread
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);

    //Start the server

    xmlrpc_c::serverAbyss::constrOpt opt = xmlrpc_c::serverAbyss::constrOpt();

    opt.registryP(&rm->RPCManagerRegistry);
    opt.keepaliveTimeout(rm->keepalive_timeout);
    opt.keepaliveMaxConn(rm->keepalive_max_conn);
    opt.timeout(rm->timeout);
    opt.socketFd(rm->socket_fd);

    if (!rm->xml_log_file.empty())
    {
        opt.logFileName(rm->xml_log_file);
    }

    opt.maxConn(rm->max_conn);
    opt.maxConnBacklog(rm->max_conn_backlog);

    rm->AbyssServer = new xmlrpc_c::serverAbyss(opt);

    rm->AbyssServer->run();

    return 0;
}    


void RPCManager::register_xml_methods(){

    //Fass& fass = Fass::instance();

    // methods go here
    // TODO: new methods + redirect for all other
    // System Methods
    xmlrpc_c::defaultMethodPtr one_proxy(new RequestOneProxy());
    xmlrpc_c::methodPtr system_version(new SystemVersion());

    // add to registry
    RPCManagerRegistry.addMethod("fass.system.version", system_version);
    RPCManagerRegistry.setDefaultMethod(one_proxy);


};

bool RPCManager::start(){

    //cout << "Starting RPC Manager..." << endl;

    pthread_attr_t  pattr;
    ostringstream   oss;

    FassLog::log("RPCM",Log::INFO,"Starting RPC Manager...");

    int rc = setup_socket();

    if ( rc != 0 )
    {
        return false;
    }

    register_xml_methods();

    // Action loop
    //pthread_attr_init (&pattr);
    //pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);

    //pthread_create(&rm_thread,&pattr,rm_action_loop,(void *)this);

    // Server loop
    pthread_attr_init (&pattr);
    pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);

    oss << "Starting XML-RPC server, port " << port << " ...";
    FassLog::log("RPCM",Log::INFO,oss);

    pthread_create(&rm_xml_server_thread,&pattr,rm_xml_server_loop,(void *)this);



    return true;
};


bool RPCManager::setup_socket()
{
    int                 rc;
    int                 yes = 1;
    struct sockaddr_in  rm_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if ( socket_fd == -1 )
    {
        ostringstream oss;

        oss << "Cannot open server socket: " << strerror(errno);
        FassLog::log("RPCM",Log::ERROR,oss);

        return false;
    }

    rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if ( rc == -1 )
    {
        ostringstream oss;

        oss << "Cannot set socket options: " << strerror(errno);
        FassLog::log("RPCM",Log::ERROR,oss);

        close(socket_fd);

        return false;
    }
    fcntl(socket_fd,F_SETFD,FD_CLOEXEC); // set close-on-exec

    rm_addr.sin_family      = AF_INET;
    // converts the unsigned short integer hostshort from host byte order to network byte order
    rm_addr.sin_port        = htons(port);

    // converts the Internet host address cp from IPv4 numbers-and-dots notation 
    // into binary data in network byte order
    rc = inet_aton(listen_address.c_str(), &rm_addr.sin_addr);

    if ( rc == 0 )
    {
        ostringstream oss;

        oss << "Invalid listen address: " << listen_address;
        FassLog::log("RPCM",Log::ERROR,oss);

        close(socket_fd);

        return false;
    }


    rc = bind(socket_fd,(struct sockaddr *) &(rm_addr),sizeof(struct sockaddr));

    if ( rc == -1)
    {
        ostringstream oss;

        oss << "Cannot bind to " << listen_address << ":" << port << " : " << strerror(errno);
        FassLog::log("RPCM",Log::ERROR,oss);

        close(socket_fd);

        return false;
    }

    return 0;
};
