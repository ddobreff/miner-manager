#!/bin/bash
# Adjust AMD GPU fan speeds according to card temperatures.
# Note that this script is intended to be run routinely from root's crontab.

# Set your TARGET temperature for all GPUs. If mining, this will realistically be on a scale from
# 50 to 70 degrees Celcius according to the temperatures you will put up with and the noise you
# will put up with.
# Copyright somewhere on the world wide web :)

function checkgputemp {
        THERMOSTAT=65; # edit this to match target temperature
        TEMP=$1;
        PERCENT=`bc <<< "scale=2; ($TEMP/$THERMOSTAT)*100"`;
        echo "$PERCENT";
}

# Our fan speed (a value from 0-255) can sensibly be adjusted within a range of 50-250, where 50
# is all but silent (20% of fan capacity) and 250 is full blast (air and noise alike). Under the
# normal case, we'd like it to be between 100 and 200 (40% to 80%) of fan capacity while mining.

function decidefanspeed {
        TMPP=$1;
        TMPI=`echo $1 | cut -d \. -f 1| bc`;
        FAN=$2
        MINFAN=$3
        NEWFAN="$FAN";
        [ "$TMPI" -lt 90 ] && NEWFAN=`expr "$FAN" - 30`;
        [ "$TMPI" -lt 92 ] && NEWFAN=`expr "$FAN" - 20`;
        [ "$TMPI" -lt 96 ] && NEWFAN=`expr "$FAN" - 10`;
        [ "$TMPI" -gt 103 ] && NEWFAN=`expr "$FAN" + 10`;
        [ "$TMPI" -gt 106 ] && NEWFAN=`expr "$FAN" + 20`;
        [ "$TMPI" -gt 108 ] && NEWFAN=`expr "$FAN" + 30`;
        [ "$TMPI" -gt 110 ] && NEWFAN=200;
        [ "$TMPI" -gt 120 ] && NEWFAN=250;
        [ "$NEWFAN" -gt 250 ] && NEWFAN=250;
        [ "$NEWFAN" -lt 50 ] && NEWFAN=50;
if [ $NEWFAN -le $MINFAN ]; then
        NEWFAN=$MINFAN
        echo "$NEWFAN";
else
        echo "$NEWFAN";
fi
}

x="0";
for GPU in  /sys/class/drm/card?/device/hwmon/hwmon?/ ; do
        [ -r $GPU/temp1_input ] && [ -w $GPU/pwm1 ] && \
        MINFANRAW=65 # edit this to match real minimal fanspeed \
        MINFAN=$(bc <<< "scale=2; ($MINFANRAW*255/100)" | cut -d \. -f 1) \
        FAN=`cat $GPU/pwm1` && TMP=$((`cat $GPU/temp1_input`/1000)) && \
        FANP=`bc <<< "scale=2; ($FAN/255)*100"` && TMPP=$(checkgputemp "$TMP") && NEWFAN=$(decidefanspeed "$TMPP" "$FAN" "$MINFAN") && \
        NFANP=`bc <<< "scale=2; ($NEWFAN/255)*100"` && \
        echo -n "GPU $x has temp $TMP" && echo -n $'\xc2\xb0'C && echo " ($TMPP%); set fan speed from $FAN/255 ($FANP%) to $NEWFAN/255 ($NFANP%)" && echo "$NEWFAN" > $GPU/pwm1;
x="$(($x + 1))"
done
