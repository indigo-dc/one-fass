/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef REQUEST_ONEPROXY_H
#define REQUEST_ONEPROXY_H

#include "Request.h"

using namespace std;

/* This class does not inherit from Request, 
   it is a simple wrapper to forward to ONE endpoint
   all requests not implemented by FASS */

//class RequestOneProxy: public xmlrpc_c::method
class RequestOneProxy: public Request
{

// In order to use private and protected members of class Request

public:

    RequestOneProxy( const string& method_name = "OneProxy",
                       const string& help = "Forwards all unhandled methods to ONE endpoint",
                       const string& params = "")
        :Request(method_name,params,help)
    {};
    

    ~RequestOneProxy(){};

protected:

    // placeholders to implement virtual methods of parent class
    void request_execute(xmlrpc_c::paramList const& _paramList,
                                 RequestAttributes& att) {};
    void execute(xmlrpc_c::paramList const& _paramList,
                        xmlrpc_c::value * const _retval) {};                    

    // real execute function 
    void execute(const string& methodName, xmlrpc_c::paramList const& _paramList,
                                 xmlrpc_c::value * retval);
};

#endif
