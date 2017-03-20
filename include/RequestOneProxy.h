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

#ifndef REQUEST_ONEPROXY_H
#define REQUEST_ONEPROXY_H

#include "Request.h"
#include "XMLRPCClient.h"
#include "Log.h"

using namespace std;

/** This class does not inherit from Request, 
   it is a simple wrapper to forward to ONE endpoint
   all requests not implemented by FASS */

//class RequestOneProxy: public xmlrpc_c::method
class RequestOneProxy: public Request
{

/// In order to use private and protected members of class Request

public:

    RequestOneProxy( const string& _one_endpoint, const long int _message_size, 
                     const string& _format_str, 
                     const int _timeout,
                     const string& method_name = "OneProxy",
                     const string& help = "Forwards all unhandled methods to ONE endpoint",
                     const string& params = "")
        :Request(method_name,_format_str,params,help),one_endpoint(_one_endpoint),message_size(_message_size),format_str(_format_str),timeout(_timeout)
    {};
    

    ~RequestOneProxy(){};

private:
    string one_endpoint;    
    long int message_size;
    string format_str;    
    int timeout;
  
protected:
    /// placeholders to implement virtual methods of parent class
    void request_execute(xmlrpc_c::paramList const& _paramList,
                                 RequestAttributes& att) {};
    void execute(xmlrpc_c::paramList const& _paramList,
                        xmlrpc_c::value * const _retval) {};                    

    /// real execute function 
    void execute(const string& methodName, xmlrpc_c::paramList const& _paramList,
                                 xmlrpc_c::value * const retval);


};

#endif
