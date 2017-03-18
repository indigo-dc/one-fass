/**
 * Copyright © 2017 INFN Torino - INDIGO-DataCloud
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "InitShares.h"
#include <fstream>

using namespace std;

template <typename T>
boost::program_options::typed_value<T>* make_value(T* store_to)
{
  return boost::program_options::value<T>(store_to);
}

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
  return user(boost::lexical_cast<unsigned short>(tokens[0]),
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
   
  std::vector<user> users;
  std::transform(user_configs.begin(), user_configs.end(),
                  std::back_inserter(users),
                  make_user);
  
    return true;
}

const char * FassInitShares::shares_name="shares.conf";

