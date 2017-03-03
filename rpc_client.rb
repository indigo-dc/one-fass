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
#server = XMLRPC::Client.new(host="http://localhost", path="/RPC2", port=2633, user="oneadmin", password="nebula")
server = XMLRPC::Client.new("localhost", "/RPC2", 2637)
begin
  #param = server.call("fass.system.version", "","")
  #param = server.call("one.clusterpool.info","","");
  #param = server.call("one.clusterpool.info","oneadmin:nebula");
  #param = server.call("one.vmpool.info","oneadmin:nebula",-2,-1,-1,-1);
  #param = server.call("one.system.config","oneadmin:nebula");
  #param = server.call("one.system.version", "oneadmin:nebula","", "");
  #param = server.call("one.system.version",10);

  #puts "#{server}"
  puts "#{param}"
rescue XMLRPC::FaultException => e
  puts "Error:"
  puts e.faultCode
  puts e.faultString
end
