/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */
#include "RequestSystem.h"
#include "Fass.h"
#include "FassLog.h"

using namespace std;

//void SystemVersion::request_execute(xmlrpc_c::paramList const& paramList,
//                                 RequestAttributes& att)
void SystemVersion::request_execute(RequestAttributes& att)
{

    success_response(Fass::instance().version(), att);
    
    return;
}

