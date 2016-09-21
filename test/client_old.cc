/* This is to test the XML-RPC server functionality */

#include <cstdlib>
#include <string>
#include <iostream>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

using namespace std;

int main(int argc, char **) {

    if (argc-1 > 0) {
        cerr << "This program has no arguments" << endl;
        exit(1);
    }

    try {
        string const serverUrl("http://localhost:2635/RPC2");
        //string const methodName("one.vmpool.info");
        string const methodName("fass.system.version");

        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;
        //string dummy="dummy";
        //int dummy=1;
        
        //myClient.call(serverUrl, methodName, "iiii", &result, -2, -1, -1, -1);
        myClient.call(serverUrl, methodName, "s", &result, "dummy");

        //string const val = xmlrpc_c::value_string(result);
        // Assume the method returned an integer; throws error if not

        //cout << "Result of RPC: " << val << endl;

    } catch (exception const& e) {
        cerr << "Client threw error: " << e.what() << endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }

    return 0;
}
