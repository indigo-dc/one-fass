/**
 * RequestOneProxy.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */


#include "RequestOneProxy.h"
#include "Fass.h"

using namespace std;

void RequestOneProxy::execute(                                                                     
        const string& _method_name,                                                        
        xmlrpc_c::paramList const& _paramList,                                            
	xmlrpc_c::value * const _retval)
//        xmlrpc_c::value *   const  _retval)      
{   

    /** XML-RPC Client */
 
XMLRPCClient * client;
try
    {
    string one_xmlrpc = "http://localhost:2633/RPC2";
    long long    message_size = 1073741824;
    unsigned int timeout = 60;


    client = XMLRPCClient::initialize("oneadmin:opennebula", one_xmlrpc, message_size, timeout);
    
    //XMLRPCClient::initialize("", one_xmlrpc, message_size, timeout);



    ostringstream oss;
    oss.str("");

    oss << "XML-RPC client using " << (XMLRPCClient::client())->get_message_size()
        << " bytes for response buffer.\n";

    FassLog::log("SCHED", Log::INFO, oss);
    }
    catch(runtime_error & err)
    {
    throw;
    }
                                                                                       
   RequestAttributes att;

        try
        {

            //XMLRPCClient * client = XMLRPCClient::client();

            //xmlrpc_c::value pippo;
            //client->call(_method_name, _paramList, &pippo);
    	    

	xmlrpc_c::value pippo;
        client->call(_method_name, _paramList, &pippo);
		//stringstream oss;
            //oss << "Throwing error:" << pippo.getString(1) << "\n";
            //FassLog::log("CICCIA", Log::INFO, oss);

		* _retval = pippo;
    	    //*att.retval  = pippo;
    	    //att.session = xmlrpc_c::value_string (_paramList.getString(1));
    	    //att.req_id = (reinterpret_cast<uintptr_t>(this) * rand()) % 10000;
            
	    //att.resp_obj = pippo;
	
	    //log_method_invoked(att, _paramList, format_str, _method_name, hidden_params);

            //success_response("Default!", att);
            //log_result(att, _method_name);


        }
        catch (exception const& e)
        {
            ostringstream oss;

            oss << "Cannot contact oned, will retry... Error: " << e.what();

            FassLog::log("SCHED", Log::ERROR, oss);

            att.resp_msg = "Could not connect";
            failure_response(INTERNAL, att);

        }



};   
