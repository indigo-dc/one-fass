#!/usr/bin/ruby
#
# rpc_client.rb
#
#      Author: Sara Vallero
#      Author: Valentina Zaccolo
#
require "xmlrpc/client"

require 'opennebula'

include OpenNebula

ONE_LOCATION=ENV["ONE_LOCATION"]

if !ONE_LOCATION
    RUBY_LIB_LOCATION="/usr/lib/one/ruby"
else
    RUBY_LIB_LOCATION=ONE_LOCATION+"/lib/ruby"
end

$: << RUBY_LIB_LOCATION



client = Client.new("oneadmin:opennebula", "http://localhost:2633/RPC2")
vm_pool = VirtualMachinePool.new(client, -1)

rc = vm_pool.info

if OpenNebula.is_error?(rc)
     puts rc.message
     exit -1
end

vm_pool.each do |vm|
          puts "Virtual Machine #{vm.id}"
end

