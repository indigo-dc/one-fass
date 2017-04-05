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
#include "RPCManager.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "Fass.h"
#include "FassLog.h"
#include "RequestSystem.h"
#include "RequestOneProxy.h"
#include "RequestQueue.h"
#include "Request.h"

RPCManager::RPCManager(
        const string& _one_endpoint,
        const string& _port,
//        int _port,
        int _max_conn,
        int _max_conn_backlog,
        int _keepalive_timeout,
        int _keepalive_max_conn,
        int _timeout,
        const string _xml_log_file,
        const string _call_log_format,
        const string _listen_address,
        int _message_size):
            one_endpoint(_one_endpoint),
            port(_port),
            socket_fd(-1),
            max_conn(_max_conn),
            max_conn_backlog(_max_conn_backlog),
            keepalive_timeout(_keepalive_timeout),
            keepalive_max_conn(_keepalive_max_conn),
            timeout(_timeout),
            xml_log_file(_xml_log_file),
            call_log_format(_call_log_format),
            listen_address(_listen_address),
            message_size(_message_size) {
    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, message_size);

    // No Action Manager class, by now. Think if needed in the future
    // am.addListener(this);
}

extern "C" void * rm_xml_server_loop(void *arg) {
    RPCManager *    rm;

    if ( arg == 0 ) {
        return 0;
    }

    rm = static_cast<RPCManager *>(arg);
    /// Set cancel state for the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    /// Start the server

    xmlrpc_c::serverAbyss::constrOpt opt = xmlrpc_c::serverAbyss::constrOpt();

    opt.registryP(&rm->RPCManagerRegistry);
    opt.keepaliveTimeout(rm->keepalive_timeout);
    opt.keepaliveMaxConn(rm->keepalive_max_conn);
    opt.timeout(rm->timeout);
    opt.socketFd(rm->socket_fd);

    if (!rm->xml_log_file.empty()) {
        opt.logFileName(rm->xml_log_file);
    }

    opt.maxConn(rm->max_conn);
    opt.maxConnBacklog(rm->max_conn_backlog);

    rm->AbyssServer = new xmlrpc_c::serverAbyss(opt);

    rm->AbyssServer->run();

    return 0;
}

void RPCManager::register_xml_methods() {
    // Fass& fass = Fass::instance();

    // methods go here
    // TODO(valzacc or svallero): new methods

    // System Methods
    xmlrpc_c::methodPtr system_version(new SystemVersion(call_log_format));
    // Reordered queue (one.vmpool.info)
    xmlrpc_c::methodPtr reordered_queue(new ReorderedQueue(call_log_format));
    // ONE Proxy Methods
    // FassLog::log("********", Log::INFO, call_log_format);
    xmlrpc_c::defaultMethodPtr one_proxy(new RequestOneProxy(one_endpoint,
                                                             message_size,
                                                             call_log_format,
                                                             timeout));
    // add to registry
    RPCManagerRegistry.addMethod("fass.system.version", system_version);
    RPCManagerRegistry.addMethod("one.vmpool.info", reordered_queue);
    RPCManagerRegistry.setDefaultMethod(one_proxy);
}

bool RPCManager::start() {
    // cout << "Starting RPC Manager..." << endl;

    ostringstream   oss;

    FassLog::log("RPCM", Log::INFO, "Starting RPC Manager...");

    int rc = setup_socket_new();

    if ( rc != 0 ) {
        return false;
    }

    register_xml_methods();

    // Server loop
    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);

    oss << "Threading XML-RPC server, port " << port << ".";
    FassLog::log("RPCM", Log::DEBUG, oss);

    pthread_create(&rm_xml_server_thread, &pattr, rm_xml_server_loop,
                   reinterpret_cast<void *>(this));

    FassLog::log("RPCM", Log::INFO, "...started.");
    return true;
}

bool RPCManager::setup_socket() {
    int                 rc;
    int                 yes = 1;
    struct sockaddr_in  rm_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if ( socket_fd == -1 ) {
        ostringstream oss;

        oss << "Cannot open server socket: " << strerror(errno);
        FassLog::log("RPCM", Log::ERROR, oss);

        return false;
    }

    rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if ( rc == -1 ) {
        ostringstream oss;

        oss << "Cannot set socket options: " << strerror(errno);
        FassLog::log("RPCM", Log::ERROR, oss);

        close(socket_fd);

        return false;
    }
    fcntl(socket_fd, F_SETFD, FD_CLOEXEC);  // set close-on-exec
    rm_addr.sin_family      = AF_INET;
    // converts the unsigned short integer hostshort
    // from host byte order to network byte order
    rm_addr.sin_port        = htons(atoi(port.c_str()));

    // converts the Internet host address cp
    // from IPv4 numbers-and-dots notation
    // into binary data in network byte order
    rc = inet_aton(listen_address.c_str(), &rm_addr.sin_addr);

    if ( rc == 0 ) {
        ostringstream oss;

        oss << "Invalid listen address: " << listen_address;
        FassLog::log("RPCM", Log::ERROR, oss);

        close(socket_fd);

        return false;
    }


    rc = bind(socket_fd, (struct sockaddr *) &(rm_addr),
              sizeof(struct sockaddr));

    if ( rc == -1 ) {
        ostringstream oss;
        oss << "Cannot bind to " << listen_address
            << ":" << port << " : " << strerror(errno);
        FassLog::log("RPCM", Log::ERROR, oss);

        close(socket_fd);

        return false;
    }

    return 0;
}

int RPCManager::setup_socket_new() {
    int rc;
    int yes = 1;

    struct addrinfo hints = {0};
    struct addrinfo * result;

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    rc = getaddrinfo(listen_address.c_str(), port.c_str(), &hints, &result);

    if ( rc != 0 ) {
        ostringstream oss;

        oss << "Cannot open server socket: " << gai_strerror(rc);
        FassLog::log("RPCM", Log::ERROR, oss);

        return -1;
    }

    socket_fd = socket(result->ai_family, result->ai_socktype, 0);

    if ( socket_fd == -1 ) {
        ostringstream oss;

        oss << "Cannot open server socket: " << strerror(errno);
        FassLog::log("RPCM", Log::ERROR, oss);

        freeaddrinfo(result);

        return -1;
    }

    rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if ( rc == -1 ) {
        ostringstream oss;

        oss << "Cannot set socket options: " << strerror(errno);
        FassLog::log("RPCM", Log::ERROR, oss);

        close(socket_fd);

        freeaddrinfo(result);

        return -1;
    }

    fcntl(socket_fd,F_SETFD,FD_CLOEXEC);  // Close socket in MADs

    rc = bind(socket_fd, result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);

    if ( rc == -1 ) {
        ostringstream oss;

        oss << "Cannot bind to " << listen_address << ":" << port << " : "
            << strerror(errno);

        FassLog::log("RPCM", Log::ERROR, oss);

        close(socket_fd);

        return -1;
    }

    return 0;
}
