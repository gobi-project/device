require 'sqlite3'
require 'active_record'

SQLite3::Database.new('test.sqlite')
ActiveRecord::Base.establish_connection(adapter: 'sqlite3', database: 'test.sqlite')
ActiveRecord::Base.connection
ActiveRecord::Migration.verbose = false # debug messages
ActiveRecord::Migrator.migrate '.'

require 'coap'
require 'codtls'

c = CoAP::Client.new(48).use_dtls
random = Random.new

puts "start random test with socket1, socket2 and rgb light..."

suppress(Exception) do
	while true do  
	  sleep(5)	

	  state = random.rand(0..5)
	  case state
		when 0
		  value = random.rand(0...16777215)
		  c.post('aaaa::60b1:0013', 5684, '/rgb', value.to_s)
		  puts "rgb: "+value.to_s
		when 1
		  c.post('aaaa::60b1:0013', 5684, '/rgb', '0')
		  puts "rgb: 0"
		when 2
		  c.post('aaaa::60b1:00a6', 5684, '/swt', '0')
		  puts "socket1 off"
		when 3
		  c.post('aaaa::60b1:00a6', 5684, '/swt', '1')
		  puts "socket1 on"
		when 4
		  c.post('aaaa::60b1:00a7', 5684, '/swt', '0')
		  puts "socket2 off"
		when 5
		  c.post('aaaa::60b1:00a7', 5684, '/swt', '1')
		  puts "socket2 on"
	  end
	end
end