/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */
//#include "Request.h"
#include "RequestOneProxy.h"
#include "Fass.h"

using namespace std;

void RequestOneProxy::execute(                                                                     
        const string& _method_name,                                                        
        xmlrpc_c::paramList const& _paramList,                                             
        xmlrpc_c::value *   const  _retval)                                                
{                                                                                          
    RequestAttributes att;                                                                 
                                                                                           
    att.retval  = _retval;                                                                 
    att.session = xmlrpc_c::value_string (_paramList.getString(1));                        
                                                                                           
    att.req_id = (reinterpret_cast<uintptr_t>(this) * rand()) % 10000;                     
                                                                                           
    // TODO: autenticazione, solo user oneadmin -> deleghiamo tutto ad ON                  
 
    log_method_invoked(att, _paramList, format_str, _method_name, hidden_params);           

    // TODO: here should go the proxy code 
    success_response("Default!", att);
                                              
    log_result(att, _method_name);                                                           
};   
