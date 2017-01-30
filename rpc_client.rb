#
# rpc_client.rb
#
#      Author: Valentina Zaccolo
#

require 'xmlrpc/client'

# get the credentials from user auth file
@one_auth = File.read("/var/lib/one/.one/one_auth")

# get the OpenNebula server endpoint
@one_endpoint = "http://localhost:9869/RPC2"


@server = XMLRPC::Client.new2(@one_endpoint)
            
            
begin 
    response = @server.call("default", @one_auth, "")
    #response = @server.call("fass.system.version", @one_auth, "")
    
    rescue => e    
    puts "Error:"
    puts e
end

