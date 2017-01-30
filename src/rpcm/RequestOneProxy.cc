/**
 * RequestOneProxy.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "RequestOneProxy.h"
#include "Fass.h"


void RequestOneProxy::request_execute(xmlrpc_c::paramList const& _paramList,
        RequestAttributes& att)
{
    xmlrpc_c::value return_value;

    try
    {
        Client * client = Client::client();

        client->call(method, _paramList, &return_value);

        *(att.retval) = return_value;
        success_response("default", att);

}
    catch(exception const& e)
    {
        att.resp_msg = "Could not connect";
        failure_response(INTERNAL, att);
    }
}










