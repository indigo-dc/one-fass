/**
 * XMLRPCClient.h 
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 **/

#ifndef INCLUDE_XMLRPCCLIENT_H_
#define INCLUDE_XMLRPCCLIENT_H_

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <xmlrpc-c/girerr.hpp>

#include <iostream>
#include <string>
#include <sstream>

#include "FassLog.h"

using namespace std;

// =============================================================================
// Doc:
// http://xmlrpc-c.sourceforge.net/doc/#clientexamplepp
// http://xmlrpc-c.sourceforge.net/doc/libxmlrpc_client++.html#simple_client
// =============================================================================

/**
 * This class represents the connection with the core and handles the
 * xml-rpc calls.
 */
class XMLRPCClient
{
public:
    /**
     *  Singleton accessor
     */
    static XMLRPCClient * client()
    {
        return _client;
    };

    /**
     *  Singleton initializer
     */
    static XMLRPCClient * initialize(const std::string& secret,
            const std::string& endpoint, size_t message_size, unsigned int tout)
    {
        if ( _client == 0 )
        {
            _client = new XMLRPCClient(secret, endpoint, message_size, tout);
        }

        return _client;
    };

    size_t get_message_size() const
    {
        return xmlrpc_limit_get(XMLRPC_XML_SIZE_LIMIT_ID);
    };

    /**
     *  Reads ONE_AUTH from environment or its default location at
     *  $HOME/.one/one_auth
     */
    static int read_oneauth(std::string &secret, std::string& error);

    /**
     *  Performs an xmlrpc call to the initialized server and credentials.
     *  This method automatically adds the credential argument.
     *    @param method name
     *    @param format of the arguments, supported arguments are i:int, s:string
     *    and b:bool
     *    @param result to store the xmlrpc call result
     *    @param ... xmlrpc arguments
     */
    void call(const std::string &method, const std::string format,
        xmlrpc_c::value * const result, ...);

    /**
     *  Performs a xmlrpc call to the initialized server
     *    @param method name
     *    @param plist initialized param list
     *    @param result of the xmlrpc call
     */
    void call(const std::string& method, const xmlrpc_c::paramList& plist,
//         xmlrpc_c::value * result);
         xmlrpc_c::value * const result);

private:
    /**
     * Creates a new xml-rpc client with specified options.
     *
     * @param secret A string containing the ONE user:password tuple.
     * If not set, the auth. file will be assumed to be at $ONE_AUTH
     * @param endpoint Where the rpc server is listening, must be something
     * like "http://localhost:2633/RPC2". If not set, the endpoint will be set
     * to $ONE_XMLRPC.
     * @param message_size for XML elements in the client library (in bytes)
     * @throws Exception if the authorization options are invalid
     */
    XMLRPCClient(const string& secret, const string& endpoint, size_t message_size,
        unsigned int tout);

    string  one_auth;
    string  one_endpoint;

    unsigned int timeout;

    static XMLRPCClient * _client;
};

#endif /*XMLRPCCLIENT_H_*/
