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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>


//#include <unistd.h>
//#include <sys/stat.h>


using namespace std;

/* -------------------------------------------------------------------------- */


template < typename T >
boost::program_options::typed_value< T >* make_value( T* store_to )
{
  return boost::program_options::value< T >( store_to );
}

struct user 
{
  unsigned short userID;
  unsigned short groudID;
  unsigned short share;

 user (unsigned short userID,
       unsigned short groupID, 
       unsigned short share )
    : userID(userID),
      groupID(groupID),
      share (share)
  {}
};

user make_user( const std::string& user_group_share )
{
  std::vector< std::string > tokens;
  boost::split( tokens, user_group_share, boost::is_any_of( ":" ) );

  if ( 3 != tokens.size() )
  {
     using boost::program_options::validation_error;
     throw validation_error(validation_error::invalid_option_value,
                            "users.user",
                            user_group_share);
  }
  return slave(boost::lexical_cast<unsigned short>(tokens[0]),
	       boost::lexical_cast<unsigned short>(tokens[1]),
               boost::lexical_cast<unsigned short>(tokens[2]));
}

bool InitShares::load_shares(){

    std::vector<std::string> user_configs;

    cout << "Loading initial shares..." << endl;
    po::options_description desc("Allowed options");

    desc.add_options()
		("users.user", make_value(&user_configs), "initial shares")
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

