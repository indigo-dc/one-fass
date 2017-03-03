/**
 * RequestOneProxy.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */


#include "RequestOneProxy.h"
#include "Fass.h"
#include "FassLog.h"
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <typeinfo>

using namespace std;

void RequestOneProxy::execute(                                                                     
        const string& _method_name,                                                        
        xmlrpc_c::paramList const& _paramList,                                            
	xmlrpc_c::value * const _retval)
{   



    RequestAttributes att;                                                                 
                                                                                           
    att.retval  = _retval;                                                                 
    att.session = xmlrpc_c::value_string (_paramList.getString(0));                        
    
    string  uname("null");
    att.uname = uname;
    try{
       string uname = xmlrpc_c::value_string (_paramList.getString(0));    
       size_t found  = uname.find(':');
       if (found != string::npos) {
          att.uname = uname.erase(found);
       } 
    } catch (exception ){
       att.uname = "wrong format";
    }                                                                                       
    //oss << "Cannot contact oned... Error: " << e.what();
    //FassLog::log("RPCM", Log::ERROR, oss); 
    //att.uname = xmlrpc_c::value_string (_paramList.getString(0));
    att.req_id = (reinterpret_cast<uintptr_t>(this) * rand()) % 10000;                     
                                                                                           
    // TODO: autenticazione, solo user oneadmin -> deleghiamo tutto ad ON                  
    //  
    log_method_invoked(att, _paramList, format_str, _method_name, hidden_params);           
    vector<xmlrpc_c::value> values;
   
    try{
        // TODO: do not hardcode server url 
        //string const serverUrl("http://localhost:2633/RPC2");
        xmlrpc_c::clientSimple myClient;
    	xmlrpc_c::value result;
 
        FassLog::log("ONEPROXY", Log::INFO, this->one_endpoint);
        myClient.call(one_endpoint, _method_name,_paramList, &result);
        //myClient.call(serverUrl, _method_name,_paramList, &result);
    	values = xmlrpc_c::value_array(result).vectorValueValue();
    	bool   success = xmlrpc_c::value_boolean(values[0]);
        
        // one says failure
	if (!success){
    	    string message = xmlrpc_c::value_string(values[1]); 
            ostringstream oss;

            oss << "Oned returned failure... Error: " << message;
            
            FassLog::log("RPCM", Log::ERROR, oss);
            failure_response(XML_RPC_API, att);
            }
        
        // one says success
        xmlrpc_c::value val;
        const xmlrpc_c::value::type_t type = values[1].type();

        switch (type){
            case xmlrpc_c::value::TYPE_STRING: // string
               success_response(xmlrpc_c::value_string(values[1]), att);
            break; 
            case xmlrpc_c::value::TYPE_INT: // int
               success_response(xmlrpc_c::value_int(values[1]), att);
            break; 
            case xmlrpc_c::value::TYPE_BOOLEAN: // bool
               success_response(xmlrpc_c::value_boolean(values[1]), att);
            break; 
            default:
               failure_response(INTERNAL, att);
            break; 
        } 

    } catch (exception const& e){
        ostringstream oss;

        oss << "Cannot contact oned... Error: " << e.what();
        FassLog::log("RPCM", Log::ERROR, oss);
        
        ostringstream oss2;
        oss2 << "Message type is: " << values[1].type();
        FassLog::log("RPCM", Log::INFO, oss2);
        failure_response(INTERNAL, att);
        }


                                                            
    log_result(att, _method_name);                                                           

};   
