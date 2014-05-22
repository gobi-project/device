require 'sqlite3'
require 'active_record'

SQLite3::Database.new('test.sqlite')
ActiveRecord::Base.establish_connection(adapter: 'sqlite3', database: 'test.sqlite')
ActiveRecord::Base.connection
ActiveRecord::Migration.verbose = false # debug messages
ActiveRecord::Migrator.migrate '.'

require 'coap'
require 'codtls'

#  c = CoAP::Client.new(48).use_dtls
#  r = c.get('aaaa::60b1:0025', 5684, '/rgb')
#  r = c.post('aaaa::60b1:0025', 5684, '/rgb', '255')
