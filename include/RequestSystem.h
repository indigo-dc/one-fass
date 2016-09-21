/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef REQUEST_SYSTEM_H
#define REQUEST_SYSTEM_H

#include "Request.h"

using namespace std;

class RequestSystem: public Request
{
protected:
    RequestSystem( const string& method_name,
                       const string& help,
                       const string& params)
        :Request(method_name,params,help)
    {};

    ~RequestSystem(){};


    //virtual void request_execute(xmlrpc_c::paramList const& _paramList,
    //                             RequestAttributes& att) = 0;
    //virtual void request_execute(RequestAttributes& att) = 0;
};

class SystemVersion : public RequestSystem
{
public:
    SystemVersion():
        RequestSystem("SystemVersion",
                          "Returns the FASS version",
                          "A:s"){}; // returns an array (A), no input params (n)

    ~SystemVersion(){};

    //void request_execute(xmlrpc_c::paramList const& _paramList,
    //                     RequestAttributes& att);
    void request_execute(RequestAttributes& att);
};

/*
class SystemConfig : public RequestSystem
{
public:
    SystemConfig():
        RequestSystem("SystemConfig",
                          "Returns the FASS configuration",
                          "A:s")
    {};

    ~SystemConfig(){};

    void request_execute(xmlrpc_c::paramList const& _paramList,
                         RequestAttributes& att);
};	
*/

#endif
