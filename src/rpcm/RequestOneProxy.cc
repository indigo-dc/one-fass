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

#include "RequestOneProxy.h"

#include <typeinfo>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

#include "Fass.h"
#include "FassLog.h"
#include "XMLRPCClient.h"

void RequestOneProxy::execute(
        const string& _method_name,
        xmlrpc_c::paramList const& _paramList,
        xmlrpc_c::value * const _retval) {
    RequestAttributes att;

    att.retval  = _retval;
    att.session = xmlrpc_c::value_string(_paramList.getString(0));

    string  uname("null");
    att.uname = uname;
    try {
       string uname = xmlrpc_c::value_string(_paramList.getString(0));
       size_t found  = uname.find(':');
       if (found != string::npos) {
          att.uname = uname.erase(found);
       }
    } catch (exception) {
       att.uname = "wrong format";
    }
    // FassLog::log("******", Log::INFO, "Pippo!");
    // oss << "Cannot contact oned... Error: " << e.what();
    // FassLog::log("ONEPROXY", Log::ERROR, oss);
    // att.uname = xmlrpc_c::value_string (_paramList.getString(0));
    // att.req_id = (reinterpret_cast<uintptr_t>(this) * rand_r(0)) % 10000;
    // att.req_id = (reinterpret_cast<uintptr_t>(this) * rand()) % 10000;
     unsigned int seed = time(NULL);
     att.req_id = (reinterpret_cast<uintptr_t>(this) * rand_r(&seed)) % 10000;

    // FassLog::log("******", Log::INFO, "Pluto!");
    // string format_str = get_format_string();
    log_method_invoked(att, _paramList, format_str,
                       _method_name, hidden_params);
    // FassLog::log("******", Log::INFO, "Paperino!");
    vector<xmlrpc_c::value> values;

    try {
        // client initialization
        // xmlrpc_c::clientSimple myClient;
        // FassLog::log("ONEPROXY", Log::DEBUG, "Initializing client.");
        // initialize chiama il costruttore... ma si puo'?
        XMLRPCClient::initialize("", one_endpoint, message_size, timeout);
        ostringstream   oss;
        // oss << "XML-RPC client using " <<
        // (XMLRPCClient::client())->get_message_size()
        //    << " bytes for response buffer.\n";

        // FassLog::log("SARA", Log::DEBUG, oss);

        XMLRPCClient *myClient = XMLRPCClient::client();
        // first argument is auth,
        // but it is passed in the scheduler request
        // and we do not need to add it
        // FassLog::log("ONEPROXY", Log::DEBUG, "Done.");
        xmlrpc_c::value result;

        // FassLog::log("ONEPROXY", Log::INFO, this->one_endpoint);
        // myClient.call(one_endpoint, _method_name,_paramList, &result);
        try {
                // FassLog::log("ONEPROXY", Log::DEBUG, "Calling client.");
                // myClient->call_sync(_method_name,_paramList, &result);
                myClient->call_async(_method_name, _paramList, &result);
                // FassLog::log("ONEPROXY", Log::DEBUG, "Done.");
        } catch (exception const& e) {
                ostringstream   oss;
                oss << "Exception raised: " << e.what();

                FassLog::log("ONEPROXY", Log::ERROR, oss);
        }
        // myClient.call(serverUrl, _method_name,_paramList, &result);
        values = xmlrpc_c::value_array(result).vectorValueValue();
        bool   success = xmlrpc_c::value_boolean(values[0]);

        // one says failure
        if (!success) {
            string message = xmlrpc_c::value_string(values[1]);
            ostringstream oss;

            oss << "Oned returned failure... Error: " << message;

            FassLog::log("ONEPROXY", Log::ERROR, oss);
            failure_response(XML_RPC_API, att);
            }

        // one says success
        xmlrpc_c::value val;
        const xmlrpc_c::value::type_t type = values[1].type();

        switch (type) {
            case xmlrpc_c::value::TYPE_STRING:  // string
               success_response(xmlrpc_c::value_string(values[1]), att);
            break;
            case xmlrpc_c::value::TYPE_INT:  // int
               success_response(xmlrpc_c::value_int(values[1]), att);
            break;
            case xmlrpc_c::value::TYPE_BOOLEAN:  // bool
               success_response(xmlrpc_c::value_boolean(values[1]), att);
            break;
            default:
               failure_response(INTERNAL, att);
            break;
        }

    // delete myClient;
    } catch (exception const& e) {
        ostringstream oss;

        oss << "Cannot contact oned... Error: " << e.what();
        FassLog::log("ONEPROXY", Log::ERROR, oss);

        ostringstream oss2;
        oss2 << "Message type is: " << values[1].type();
        FassLog::log("ONEPROXY", Log::INFO, oss2);
        failure_response(INTERNAL, att);
        }

log_result(att, _method_name);
}
