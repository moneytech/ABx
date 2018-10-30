--------------------------------------------------------------------------------
-- General Settings ------------------------------------------------------------
--------------------------------------------------------------------------------

server_id = "548350e7-5919-4bcf-a885-2d2888db05ca"
location = "AT"
server_name = "Admin"

admin_ip = ""         -- Listen on all
admin_host = ""       -- emtpy use same host as for login
admin_port = 443

server_key = "server.key"
server_cert = "server.crt"

-- Thread pool size
num_threads = 4
root_dir = "c:/Users/Stefan Ascher/Documents/Visual Studio 2015/Projects/ABx/Bin/admin/root"
session_lifetime = 1000 * 60 * 10

require("config/data_server")
require("config/msg_server")