/**
 * InitShares.cc
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "InitShares.h"
#include "FassLog.h"

//#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

//#include <unistd.h>
//#include <sys/stat.h>


using namespace std;

/* -------------------------------------------------------------------------- */

bool InitShares::load_shares(){

    cout << "Loading initial shares..." << endl;
    po::options_description desc("Allowed options");

    desc.add_options()
		("init_shares", po::value<string>()->default_value("0:0:0"), "initial shares")
		;

    /// Read the initial shares file
    ifstream settings_file(shares_file.c_str());

    /// Clear the map
    vm = po::variables_map();

    try{
        po::store(po::parse_shares_file(initset_file , desc), vm);
    } catch(const exception& re){
        cerr << "Error: " << re.what() << endl;
        return false;
    }

    po::notify(vm);     
    
    return true;
}

const char * FassInitShares::shares_name="shares.conf";

