# dns_update

dns_update is tool for dynamic dns name, it's a dns server and dns update client.
for example, you have domain name call abc.com, and you want to resolve home.abc.com to 
you home ip adress( your family ip address may change after you reboot your router),
you should add ns type record, home.abc.com, set value is ns.abc.com(this must be domain,
not a ip address), and you deploy this dns server program in in ns.abc.com.
after you compile this program , two program generated, call dns and dsn_update,
dns is dns server , and dsn_update is update client.


# config
## server
dns.cfg is server config file

[dns]

port = 20053

auth_key = 1Y8IZwoC


port this the udp port to receive client udp update, auth_key is authentication key of update command.

## client
the param of client dsn_update 
./dns_update server:port auth_key  domain

in this case the command line should be:
./dns_update ns.ab.com:20053  1Y8IZwoC home.abc.com





