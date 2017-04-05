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

#include <json/json.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>

// #include <Poco/Util/Application.h>
// #include <Poco/Net/HTTPClientSession.h>
// #include <Poco/Net/HTTPRequest.h>
// #include <Poco/Net/HTTPResponse.h>
// #include <Poco/StreamCopier.h>
// #include <Poco/Path.h>
// #include <Poco/URI.h>
// #include <Poco/Exception.h>

// using namespace Poco::Net;
// using namespace Poco;

using boost::asio::ip::tcp;

int main(int argc, char **argv) {
  printf("This is a test to query InfluxDb!!!\n");

  // curl -G 'http://localhost:8086/query?pretty=true'
  // --data-urlencode "db=mydb" --data-urlencode
  // "q=SELECT \"value\" FROM \"cpu_load_short\"
  // WHERE \"region\"='us-west'"

  try {
    boost::asio::io_service io_service;

  // Get a list of endpoints corresponding to the server name.
  tcp::resolver resolver(io_service);
  tcp::resolver::query query("localhost", "8086");
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

  // Try each endpoint until we successfully establish a connection.
  tcp::socket socket(io_service);
  boost::asio::connect(socket, endpoint_iterator);

  // Form the request. We specify the "Connection: close" header so that the
  // server will close the socket after transmitting the response. This will
  // allow us to treat all data up until the EOF as the content.

  boost::asio::streambuf request;
  std::ostream request_stream(&request);
  // request_stream << "GET " << "/ping" << " HTTP/1.0\r\n";
  // request_stream << "GET " <<
  // "/query?pretty=true&db=mydb&q=show%20databases"
  // << " HTTP/1.0\r\n";
  std::string content("zeppola,user=pippo,group=pluto value=0.3");
  request_stream << "POST "
  << "/write?db=mydb"
  << " HTTP/1.1\r\n";
  request_stream << "Host: " << "localhost:8086" << "\r\n";
  request_stream << "Content-Type: application/x-www-form-urlencoded \r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Content-Length: " << content.length() << "\r\n";
  request_stream << "Connection: close\r\n\r\n";
  request_stream << content;

  // Send the request.
  boost::asio::write(socket, request);

  // Read the response status line. The response streambuf will automatically
  // grow to accommodate the entire line. The growth may be limited by passing
  // a maximum size to the streambuf constructor.
  boost::asio::streambuf response;
  boost::asio::read_until(socket, response, "\r\n");

  // Check that response is OK.
  std::istream response_stream(&response);
  std::string http_version;
  response_stream >> http_version;
  unsigned int status_code;
  response_stream >> status_code;
  std::string status_message;
  std::getline(response_stream, status_message);
  if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            return 1;
  }
  if (status_code != 200) {
        std::cout << "Response returned with status code "
                      << status_code << "\n";
        std::cout << status_message << "\n";
        std::cout << response_stream.rdbuf() << "\n";
            return 1;
  }

  // Read the response headers, which are terminated by a blank line.
  boost::asio::read_until(socket, response, "\r\n\r\n");

  // Process the response headers.
  std::string header;
  while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";
  std::cout << "\n";

  // Write whatever content we already have to output.
  if (response.size() > 0)  std::cout << &response;

  // Read until EOF, writing data to output as we go.
  boost::system::error_code error;
  while (boost::asio::read(socket, response,
                            boost::asio::transfer_at_least(1), error))
           std::cout << &response;
  if (error != boost::asio::error::eof) {
        throw boost::system::system_error(error);
  }
  } catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << "\n";
  }
  return 0;
}
