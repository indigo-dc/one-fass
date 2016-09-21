# To test XMLRPC server 

require "xmlrpc/client"

#server = XMLRPC::Client.new("localhost", "/RPC2", 2635)
server = XMLRPC::Client.new("172.17.0.2", "/RPC2", 2635)
begin
  #param = server.call("fass.system.version", "pippo")
  param = server.call("piripicchiolo","pippo","pluto")
  puts "#{param}"
rescue XMLRPC::FaultException => e
  puts "Error:"
  puts e.faultCode
  puts e.faultString
end
