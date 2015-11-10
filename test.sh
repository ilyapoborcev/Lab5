sudo echo "delay = 2"
sudo echo "2" > /dev/hello
echo "Wait for 11 sec"
sleep 11
dmesg | tail 

sudo echo "0" > /dev/hello
dmesg | tail
