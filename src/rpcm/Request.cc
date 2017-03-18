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

#include "Request.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>

#include "FassLog.h"

string Request::format_str;
 
using namespace std;

void Request::execute(
        xmlrpc_c::paramList const& _paramList,
        xmlrpc_c::value *   const  _retval)
{
    RequestAttributes att;
    // user attributes
    struct passwd *pw;
    uid_t uid;
    gid_t gid;
    uid = geteuid (); 
    gid = getegid ();
    pw = getpwuid (uid);
    att.uid = uid;
    att.gid = gid;
    att.uname = pw->pw_name; 
    
    att.retval  = _retval;
    att.session = xmlrpc_c::value_string (_paramList.getString(0));

    att.req_id = (reinterpret_cast<uintptr_t>(this) * rand()) % 10000;


    log_method_invoked(att, _paramList, format_str, method_name, hidden_params);
   
    request_execute(_paramList, att);  
    log_result(att, method_name);
};

void Request::log_method_invoked(const RequestAttributes& att,
        const xmlrpc_c::paramList&  paramList, const string& format_str,
        const std::string& method_name, const std::set<int>& hidden_params)
{
    std::ostringstream oss;

    for (unsigned int j = 0 ;j < format_str.length() - 1; j++ )
    {
        if (format_str[j] != '%')
        {
            oss << format_str[j];
        }
        else
        {
            char mod = format_str[j+1];

            switch(mod)
            {
                case '%':
                    oss << "%";
                break;

                case 'i':
                    oss << att.req_id;
                break;

                case 'u':
                    oss << att.uid;
                break;

                case 'U':
                    oss << att.uname;
                break;

                case 'g':
                    oss << att.gid;
                break;

                case 'G':
                    oss << att.gname;
                break;

                case 'p':
                    oss << att.password;
                break;

                case 'a':
                    oss << att.session;
                break;

                case 'm':
                    oss << method_name;
                break;

                case 'l':
                    for (unsigned int i=1; i<paramList.size(); i++)
                    {
                        if ( hidden_params.count(i) == 1 )
                        {
                            oss << ", ****";
                        }
                        else
                        {
                            log_xmlrpc_value(paramList[i], oss);
                        }
                    }
                break;

                default:
                    oss << format_str[j] << format_str[j+1];
                break;
            }

            j = j+1;
        }
    }

    FassLog::log("REQUEST", Log::DDEBUG, oss);
}

void Request::log_xmlrpc_value(const xmlrpc_c::value& v, std::ostringstream& oss)
{
    size_t st_limit = 20;
    size_t st_newline;

    switch (v.type())
    {
        case xmlrpc_c::value::TYPE_INT:
            oss << ", " << static_cast<int>(xmlrpc_c::value_int(v));
            break;
        case xmlrpc_c::value::TYPE_BOOLEAN:
            oss << ", ";

            if ( static_cast<bool>(xmlrpc_c::value_boolean(v)) )
            {
                oss << "true";
            }
            else
            {
                oss << "false";
            }

            break;
        case xmlrpc_c::value::TYPE_STRING:
            st_newline =
                    static_cast<string>(xmlrpc_c::value_string(v)).find("\n");

            if ( st_newline < st_limit )
            {
                st_limit = st_newline;
            }

            oss << ", \"" <<
                static_cast<string>(xmlrpc_c::value_string(v)).substr(0,st_limit);

            if ( static_cast<string>(xmlrpc_c::value_string(v)).size() > st_limit )
            {
                oss << "...";
            }

            oss << "\"";
            break;
        case xmlrpc_c::value::TYPE_DOUBLE:
            oss << ", "
                << static_cast<double>(xmlrpc_c::value_double(v));
            break;
        default:
            oss  << ", unknown param type";
            break;
    }
}

void Request::log_result(const RequestAttributes& att, const string& method_name)
{
    std::ostringstream oss;

    oss << "Req:" << att.req_id << " UNAME:";

    //if ( att.uid != -1 )
    //{
    //    oss << att.uid;
    //}
    //else
    //{
    //    oss << "-";
    //}

    oss << att.uname;
    oss << " " << method_name << " result ";

    xmlrpc_c::value_array array1(*att.retval);
    vector<xmlrpc_c::value> const vvalue(array1.vectorValueValue());

    if ( static_cast<bool>(xmlrpc_c::value_boolean(vvalue[0])) )
    {
        oss << "SUCCESS";

        for (unsigned int i=1; i<vvalue.size()-1; i++)
        {
            log_xmlrpc_value(vvalue[i], oss);
        }

        FassLog::log("REQUEST", Log::DDEBUG, oss);
    }
    else
    {
        oss << "FAILURE "
            << static_cast<string>(xmlrpc_c::value_string(vvalue[1]));

        FassLog::log("REQUEST", Log::ERROR, oss);
    }
}


void Request::success_response(const string& val, RequestAttributes& att)
{
    vector<xmlrpc_c::value> arrayData;

    arrayData.push_back(xmlrpc_c::value_boolean(true));
    arrayData.push_back(xmlrpc_c::value_string(val));
    arrayData.push_back(xmlrpc_c::value_int(SUCCESS));

    xmlrpc_c::value_array arrayresult(arrayData);

    *(att.retval) = arrayresult;
}

void Request::success_response(const int& val, RequestAttributes& att)
{
    vector<xmlrpc_c::value> arrayData;

    arrayData.push_back(xmlrpc_c::value_boolean(true));
    arrayData.push_back(xmlrpc_c::value_int(val));
    arrayData.push_back(xmlrpc_c::value_int(SUCCESS));

    xmlrpc_c::value_array arrayresult(arrayData);

    *(att.retval) = arrayresult;
}

void Request::success_response(const bool& val, RequestAttributes& att)
{
    vector<xmlrpc_c::value> arrayData;

    arrayData.push_back(xmlrpc_c::value_boolean(true));
    arrayData.push_back(xmlrpc_c::value_boolean(val));
    arrayData.push_back(xmlrpc_c::value_int(SUCCESS));

    xmlrpc_c::value_array arrayresult(arrayData);

    *(att.retval) = arrayresult;
}

void Request::failure_response(ErrorCode ec, RequestAttributes& att)
{
    vector<xmlrpc_c::value> arrayData;

    arrayData.push_back(xmlrpc_c::value_boolean(false));
    arrayData.push_back(xmlrpc_c::value_int(ec));

    xmlrpc_c::value_array arrayresult(arrayData);

    *(att.retval) = arrayresult;
}
