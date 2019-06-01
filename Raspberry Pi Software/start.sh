#!/bin/sh

#sleep 3
#sudo rfkill unblock bluetooth
#sudo hciconfig hci0 up
#sleep 1


#echo "NOTEST" > /home/pi/start.txt

gpio mode 0 in
gpio mode 0 up
gpio mode 1 out
#gpio mode 2 out
gpio write 1 1
#gpio write 2 1

#sudo /home/pi/soccer/soccer SLAVE PI_TWO &

pin17=$(gpio read 0)
while [ "$pin17" = "1" ]
do

	#sudo /home/pi/soccer/soccer
	#sudo /home/pi/soccer/soccer SLAVE PI_TWO
	sleep 1
	pin17=$(gpio read 0)
done

sudo poweroff &
exit 0
