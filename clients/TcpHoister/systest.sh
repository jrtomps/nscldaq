./systest &
./systest_server&
./TcpHoist localhost

sleep 1

killall systest_server
killall systest


exit 0