/**
 * XMLRPCClient.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "XMLRPCClient.h"
#include "FassLog.h"
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <typeinfo>

#include <fstream>
#include <pwd.h>
#include <stdlib.h>
#include <stdexcept>
#include <set>

#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <sstream>

#include <unistd.h>
#include <sys/types.h>

XMLRPCClient * XMLRPCClient::_client = 0;

XMLRPCClient::XMLRPCClient(const string& secret, const string& endpoint, size_t message_size, unsigned int tout)
{
    //string error;
    //char * xmlrpc_env;

    _auth = secret;

    _endpoint = endpoint;

    //xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, message_size);

    _timeout = tout * 1000;

    //FassLog::log("CLIENT", Log::DDDEBUG, _endpoint);
};



void XMLRPCClient::call_async(const std::string& method, const xmlrpc_c::paramList& plist, xmlrpc_c::value * const result)
{
    xmlrpc_c::clientXmlTransport_curl ctrans;
    xmlrpc_c::client_xml              client(&ctrans);

    xmlrpc_c::rpcPtr rpc(method, plist);
    xmlrpc_c::carriageParm_curl0 cparam(_endpoint);

    rpc->start(&client, &cparam);

    client.finishAsync(xmlrpc_c::timeout(_timeout));

    if (!rpc->isFinished())
    {
        rpc->finishErr(girerr::error("XMLRPC method " + method +
            " timeout, resetting call"));
    }

    if (rpc->isSuccessful())
    {
        *result = rpc->getResult();
    }
    else
    {
        xmlrpc_c::fault failure = rpc->getFault();

        girerr::error(failure.getDescription());
    }

};

void XMLRPCClient::call_sync(const std::string& method, const xmlrpc_c::paramList& plist, xmlrpc_c::value  * const result)
{
    FassLog::log("CLIENT", Log::INFO, _endpoint);
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value res;
    myClient.call(_endpoint, method, plist, &res); 
    * result = res;

};
