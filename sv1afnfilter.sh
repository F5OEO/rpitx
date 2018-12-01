#!/bin/sh

#COMMAND A BANDPATH FILTER FROM SV1AFN

#GPIO Declaration
if [ ! -f /sys/class/gpio/gpio26 ]; then
echo "Create GPIOs"

echo GPIO declaration
echo 26 > /sys/class/gpio/export
echo 19 > /sys/class/gpio/export
echo 13 > /sys/class/gpio/export
echo 6  > /sys/class/gpio/export


#GPIO out
echo GPIO Mode OUT
echo out > /sys/class/gpio/gpio26/direction
echo out > /sys/class/gpio/gpio19/direction
echo out > /sys/class/gpio/gpio13/direction
echo out > /sys/class/gpio/gpio6/direction


fi

#Initialization : All at zero (disable filter)
reset_all()
{
	echo 0 > /sys/class/gpio/gpio26/value
	echo 0 > /sys/class/gpio/gpio19/value
	echo 0 > /sys/class/gpio/gpio13/value
	echo 0 > /sys/class/gpio/gpio6/value
}

reset_all
case "$1" in
"10m")
	echo 1 > /sys/class/gpio/gpio26/value
        ;;
"15m")
    echo 1 > /sys/class/gpio/gpio19/value
    ;;
"20m")
    echo 1 > /sys/class/gpio/gpio13/value
    ;;
"40m")
    echo 1 > /sys/class/gpio/gpio6/value
    ;;

*)
     ;;
esac
