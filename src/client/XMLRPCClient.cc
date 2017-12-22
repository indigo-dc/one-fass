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

#include "XMLRPCClient.h"

#include <pwd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <stdexcept>
#include <set>
#include <sstream>
#include <typeinfo>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

#include "FassLog.h"

XMLRPCClient * XMLRPCClient::_client = 0;

XMLRPCClient::XMLRPCClient(const string& secret,
                           const string& endpoint,
                           size_t message_size,
                           unsigned int tout) {
    // string error;
    // char * xmlrpc_env;

    _auth = secret;

    _endpoint = endpoint;

    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, message_size);

    _timeout = tout * 1000;

    // FassLog::log("CLIENT", Log::DDDEBUG, _endpoint);
}

void XMLRPCClient::call_async(const std::string& method,
                              const xmlrpc_c::paramList& plist,
                              xmlrpc_c::value * const result) {
    xmlrpc_c::clientXmlTransport_curl ctrans;
    xmlrpc_c::client_xml              client(&ctrans);

    xmlrpc_c::rpcPtr rpc(method, plist);
    xmlrpc_c::carriageParm_curl0 cparam(_endpoint);

    rpc->start(&client, &cparam);

    client.finishAsync(xmlrpc_c::timeout(_timeout));

    if (!rpc->isFinished()) {
        rpc->finishErr(girerr::error("XMLRPC method " + method +
            " timeout, resetting call"));
    }
    if (rpc->isSuccessful()) {
        *result = rpc->getResult();
    } else {
        xmlrpc_c::fault failure = rpc->getFault();

        girerr::error(failure.getDescription());
    }
}

void XMLRPCClient::call_sync(const std::string& method,
                             const xmlrpc_c::paramList& plist,
                             xmlrpc_c::value  * const result) {
    FassLog::log("CLIENT", Log::INFO, _endpoint);
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value res;
    myClient.call(_endpoint, method, plist, &res);
    * result = res;
}

void XMLRPCClient::call(const std::string& method,
                        const xmlrpc_c::paramList& plist,
                        xmlrpc_c::value * const result) {
    // FassLog::log("SARA", Log::INFO, _auth);
    // appends the OpenNebula secret to the list of parameters
    xmlrpc_c::paramList s_plist;
    s_plist.add(xmlrpc_c::value_string(_auth));

    for (unsigned i = 0; i < plist.size(); i++) {
        s_plist.addc(plist[i]);
    }


    xmlrpc_c::value res;
    XMLRPCClient::call_async(method, s_plist, result);
    // XMLRPCClient::call_sync(method, s_plist, &res);
    // * result = res;
}
