#!/usr/bin/ruby
#
# rpc_client.rb
#
#      Author: Sara Vallero
#      Author: Valentina Zaccolo
#
require "xmlrpc/client"

# get the credentials from user auth file
# @one_auth = File.read("/var/lib/one/.one/one_auth")
#
# # get the OpenNebula server endpoint
# @one_endpoint = "http://localhost:2633/RPC2"
##
# @server = XMLRPC::Client.new2(@one_endpoint)
     
         
#         begin 
#	param = @server.call("one.vmpool.info", @one_auth, "")
#	puts "#{param}"
#	end
#@one_auth = File.read("/var/lib/one/.one/one_auth")

#server = XMLRPC::Client.new("localhost", "/RPC2", 2633)
server = XMLRPC::Client.new("localhost", "/RPC2", 2637)
begin
#  param = server.call("fass.system.version", "","")
  param = server.call("one.vmpool.info", "", "");

  puts "#{param}"
rescue XMLRPC::FaultException => e
  puts "Error:"
  puts e.faultCode
  puts e.faultString
end
