/**
 * RequestSystem.cc
 *
 *      Author: Sara Vallero  
 *      Author: Valentina Zaccolo
 */
#include "RequestSystem.h"
#include "Fass.h"

using namespace std;

void SystemVersion::request_execute(xmlrpc_c::paramList const& paramList,
                                 RequestAttributes& att)
{
    /** Request_execute will not be executed if the session string
    * is not authenticated in Request::execute.
    * TODO: should we make the version call accessible even
    * if no user is provided? */

    success_response(Fass::instance().version(), att);

    return;
}



