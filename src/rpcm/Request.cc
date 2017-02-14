/**
 * Request.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "Request.h"
#include "FassLog.h"
#include <cstdlib>

string Request::format_str;

void Request::execute(
        xmlrpc_c::paramList const& _paramList,
        xmlrpc_c::value *   const  _retval)
{
    RequestAttributes att;

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

    FassLog::log("RPCM", Log::DEBUG, oss);
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

    oss << "Req:" << att.req_id << " UID:";

    if ( att.uid != -1 )
    {
        oss << att.uid;
    }
    else
    {
        oss << "-";
    }

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

        FassLog::log("RPCM", Log::DEBUG, oss);
    }
    else
    {
        oss << "FAILURE "
            << static_cast<string>(xmlrpc_c::value_string(vvalue[1]));

        FassLog::log("RPCM", Log::ERROR, oss);
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

void Request::failure_response(ErrorCode ec, RequestAttributes& att)
{
    vector<xmlrpc_c::value> arrayData;

    arrayData.push_back(xmlrpc_c::value_boolean(false));
    arrayData.push_back(xmlrpc_c::value_int(ec));

    xmlrpc_c::value_array arrayresult(arrayData);

    *(att.retval) = arrayresult;
}
