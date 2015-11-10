sudo echo "delay = 5"
sudo echo "5" > /sys/repeat_hello/delay
echo "Wait for 15 sec"
sleep 15
dmesg | tail 

sudo echo "delay = 2"
sudo echo "2" > /sys/repeat_hello/delay
echo "Wait for 11 sec"
sleep 10

sudo echo "0" > /sys/repeat_hello/delay
dmesg | tail
