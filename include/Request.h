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

#ifndef REQUEST_H_
#define REQUEST_H_

#include <set>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

using namespace std;

/** The Request Class represents the basic abstraction for the FASS XML-RPC API.
 *    This interface must be implemented by any XML-RPC API call */

struct RequestAttributes
{
public:
    int uid;                  /// only oneadmin allowed
    int gid;                  /// only oneadmin allowed

    string uname;             /// name of the user 
    string gname;             /// name of the user's group

    string password;          /// password of the user

    string session;           /// session from ONE XML-RPC API 
    int    req_id;            /// request ID for log messages

    //int umask;                // user umask for new objects 
    
    xmlrpc_c::value * retval; ///return value from libxmlrpc-c                         


    //PoolObjectSQL::ObjectType resp_obj; /**< object type */
    //int                       resp_id;  /**< Id of the object */
    string                    resp_msg; /**< Additional response message */
                                                                                            
}; 



class Request: public xmlrpc_c::method , public xmlrpc_c::defaultMethod
{

public:
 
    enum ErrorCode {
        SUCCESS        = 0x0000,
        //AUTHENTICATION = 0x0100,
        //AUTHORIZATION  = 0x0200,
        //NO_EXISTS      = 0x0400
        //ACTION         = 0x0800,
        XML_RPC_API    = 0x1000,
        INTERNAL       = 0x2000,
        //ALLOCATE       = 0x4000
    };



    static void set_call_log_format(const string& log_format)
    {
        format_str = log_format;
    }


protected:

    string    method_name;

    set<int> hidden_params;
    static string format_str;

    Request(const string& mn, const string& signature, const string& help):
        method_name(mn)
    {
        _signature = signature;
        _help      = help;

        hidden_params.clear();
    };

    virtual ~Request(){};
 
    /** Methods to execute the request when received at the server */

    /** Wraps the actual execution function by authorizing the user
       and calling the request_execute virtual function
       @param _paramlist list of XML parameters
       @param _retval value to be returned to the client */
    virtual void execute(xmlrpc_c::paramList const& _paramList,
        xmlrpc_c::value * const _retval);
    /** this is for default function, it's not virtual, but overwritten by RequestOneProxy */
//    void execute(const string& method_name, xmlrpc_c::paramList const& _paramList,
//        xmlrpc_c::value  _retval) {};

    void execute(const string& method_name, xmlrpc_c::paramList const& _paramList,
//        xmlrpc_c::value * _retval) {};
        xmlrpc_c::value * const _retval) {};

virtual void execute(std::string _method_name,
            xmlrpc_c::paramList paramList,
            xmlrpc_c::value *   _retval) {};


    /** Actual Execution method for the request. Must be implemented by the XML-RPC requests.
       @param _paramlist of the XML-RPC call (complete list)
       @param att the specific request attributes */
    virtual void request_execute(xmlrpc_c::paramList const& _paramList,
                                 RequestAttributes& att) = 0;

    /** Builds an XML-RPC response updating retval. 
       After calling this function the xml-rpc excute method should return
       @param val string to be returned to the client
       @param att the specific request attributes */
    void success_response(const string& val, RequestAttributes& att);
 
    /** Builds an XML-RPC response updating retval. 
       After calling this function the xml-rpc excute method should return
       @param val int to be returned to the client
       @param att the specific request attributes */
    void success_response(const int& val, RequestAttributes& att);
    
    /** Builds an XML-RPC response updating retval. 
       After calling this function the xml-rpc excute method should return
       @param val bool to be returned to the client
       @param att the specific request attributes */
    void success_response(const bool& val, RequestAttributes& att);

    /**
     *  Builds an XML-RPC response updating retval. After calling this function
     *  the xml-rpc excute method should return. A descriptive error message
     *  is constructed using att.resp_obj, att.resp_id and/or att.resp_msg and
     *  the ErrorCode
     *    @param ec error code for this call
     *    @param ra the specific request attributes
     */
    void failure_response(ErrorCode ec, RequestAttributes& ra);


    /** Logs the method invocation, including the arguments
       @param att the specific request attributes
       @param paramList list of XML parameters
       @param format_str for the log
       @param hidden_params params not to be shown */
    static void log_method_invoked(const RequestAttributes& att,
        const xmlrpc_c::paramList&  paramList, const string& format_str,
        const std::string& method_name, const std::set<int>& hidden_params);
   
    /** Logs the method result, including the output data or error message
       @param att the specific request attributes
       @param method_name that produced the error */
    static void log_result(const RequestAttributes& att,
            const std::string& method_name);
private:

    /** Formats and adds a xmlrpc_c::value to oss.
       @param v value to format
       @param oss stream to write v */
    static void log_xmlrpc_value(const xmlrpc_c::value& v, std::ostringstream& oss);



};

#endif
