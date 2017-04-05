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

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "FassDb.h"
#include "FassLog.h"

using boost::asio::ip::tcp;

bool InfluxDb::init_db() {
    ostringstream oss;
    oss << "Initializing database \"" << _dbname << "\" at http://"
        << _endpoint << ":" << _port;
    FassLog::log("INFLUXDB", Log::INFO, oss);

    string response;
    bool rc;

    // ping endpoint
    rc = InfluxDb::ping_db();
    if ( !rc ) {
        FassLog::log("INFLUXDB", Log::ERROR, "Could not ping DB!");
        return false;
    }
    // create a new database,
    // the action has no effect if the database already exists
    rc = InfluxDb::create_db();
    if ( !rc ) {
      FassLog::log("INFLUXDB", Log::ERROR, "Error creating database!");
    }
    return true;
}

bool InfluxDb::query_db(string method, string q, string &retval) {
    // for the time being we let it hardcoded
    const string protocol("HTTP/1.0");
    // FassLog::log("SARA", Log::INFO, "Starting query");

    try {
       // Taken form an example in the boost documentation
       boost::asio::io_service io_service;

       // Get a list of endpoints corresponding to the server name
       tcp::resolver resolver(io_service);
       tcp::resolver::query query(_endpoint,
                                  boost::lexical_cast<string>(_port));
       tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);


       // Try each endpoint until we successfully establish a connection
       tcp::socket socket(io_service);
       boost::asio::connect(socket, endpoint_iterator);

       // Form the request. We specify the "Connection: close" header
       // so that the server will close the socket
       // after transmitting the response. This will
       // allow us to treat all data
       // up until the EOF as the content
       boost::asio::streambuf request;
       std::ostream request_stream(&request);

       std::string my_query("");
       bool post = false;
       if (method == "WRITE") {
           post = true;
           my_query.append("/write?db=" + _dbname);
           method = "POST";
       } else {
           my_query = q;
       }
       request_stream << method << " ";
       request_stream << my_query << " " << protocol << "\r\n";
       request_stream << "Host: " << _endpoint << "\r\n";
       request_stream << "Accept: */*\r\n";
       if (post) {
           request_stream
                   << "Content-Type: application/x-www-form-urlencoded\r\n";
           request_stream << "Content-Length: " << q.length() << "\r\n";
       }
       request_stream << "Connection: close\r\n\r\n";
       if (post) request_stream << q;

       // Send the request
       boost::asio::write(socket, request);

       // Read the response status line.
       // The response streambuf will automatically
       // grow to accommodate the entire line.
       // The growth may be limited by passing
       // a maximum size to the streambuf constructor
       boost::asio::streambuf response;
       boost::asio::read_until(socket, response, "\r\n");

       // Check that response is OK
       std::istream response_stream(&response);
       std::string http_version;
       response_stream >> http_version;
       unsigned int status_code;
       response_stream >> status_code;
       std::string status_message;
       std::getline(response_stream, status_message);
       if (!response_stream
           || http_version.substr(0, 5) != "HTTP/") {
                retval = "Invalid response.";
                FassLog::log("DB-QUERY", Log::ERROR, retval);
                return false;
       }
       if (status_code != 200 && status_code != 204) {
               // ping success results in 204
               ostringstream oss_sc;
               oss_sc << "Response returned with status code " << status_code;
               // retval=oss_sc.str();
               retval = status_message;
               FassLog::log("DB-QUERY", Log::ERROR, retval);
               return false;
       }
       if (status_code == 204) {
               // ping success results in 204
               FassLog::log("DB-QUERY", Log::DDEBUG,
                            "Success!");
               return true;
       }

       // Read the response headers, which are terminated by a blank line
       boost::asio::read_until(socket, response, "\r\n\r\n");

       // Process the response headers
       ostringstream oss_h;
       std::string header;
       while (std::getline(response_stream, header)
              && header != "\r")
                oss_h  << header << "\n";
       oss_h << "\n";

       // Write whatever content we already have to output
       if (response.size() > 0)  oss_h << &response;
       FassLog::log("DB-QUERY", Log::DDEBUG, oss_h);

       // Read until EOF, writing data to output as we go
       boost::system::error_code error;
       ostringstream oss_r;
       while (boost::asio::read(socket, response,
                                boost::asio::transfer_at_least(1), error))
              oss_r << &response;
       if (error != boost::asio::error::eof)
             throw boost::system::system_error(error);
       retval = oss_r.str();
       FassLog::log("DB-QUERY", Log::DDEBUG, oss_r);
    } catch (std::exception& e) {
       ostringstream oss_e;
       oss_e << e.what();
       FassLog::log("DB-QUERY", Log::ERROR, oss_e);
       retval = oss_e.str();
       return false;
    }

    return true;
}

bool InfluxDb::ping_db() {
    string response;
    bool retval = InfluxDb::query_db("GET", "/ping", response);
    return retval;
}

bool InfluxDb::create_db() {
    string response;
    ostringstream query;
    query << "/query?pretty=false&q=create%20database%20" << _dbname;
    bool retval = InfluxDb::query_db("POST", query.str(), response);
    return retval;
}

bool InfluxDb::write_initial_shares(const float share,
                                    const string user,
                                    const string group,
                                    const int64_t timestamp) {
    FassLog::log("INFLUXDB", Log::DEBUG, "Writing initial shares.");
    // query_db(string method, string q, string &retval)
    string response;
    ostringstream query;
    query << "share,user=" << user
    << ",group=" << group
    << " value=" << share
    << " " << timestamp;
    FassLog::log("INFLUXDB", Log::DEBUG, query);
    bool retval = InfluxDb::query_db("WRITE", query.str(), response);
    return retval;
}

bool InfluxDb::write_queue(const int priority, const string user,
                           const string group, const int vmid,
                           const float cpus, const float memory,
                           const int64_t starttime, const int64_t timestamp) {
    FassLog::log("INFLUXDB", Log::INFO, "Writing queue.");

    return true;
}

bool InfluxDb::write_usage(const float cpu_usage, const float memory_usage,
                           const string user, const string group,
                           const int64_t since, const int64_t timestamp) {
    FassLog::log("INFLUXDB", Log::INFO, "Writing usage records.");

    return true;
}
