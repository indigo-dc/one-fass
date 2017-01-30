/**
 * RequestOneProxy.h
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#ifndef REQUEST_ONE_PROXY_H_
#define REQUEST_ONE_PROXY_H_

#include "Request.h"
#include "Client.h"

using namespace std;

/** This class forwards to ONE endpoint
      all requests not implemented by FASS */

class RequestOneProxy: public Request
{
public:
    RequestOneProxy(string _method): Request("RequestOneProxy", "?",
        "Forwards all unhandled methods to ONE endpoint"), method(_method)

{
        method_name = ("OneProxy." + method);
    };

    ~RequestOneProxy(){};

    void request_execute(xmlrpc_c::paramList const& _paramList,
                         RequestAttributes& att);

    void hide_argument(int arg)
    {
        hidden_params.insert(arg);
    };

private:
    string    method;
};

#endif



