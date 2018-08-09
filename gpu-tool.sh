#!/bin/bash
# Copyright 2018 Dobromir Dobrev <dobreff@gmail.com>
# This tool provides advanced monitoring for AMD and NVIDIA graphics
# For simplemining and any other linux OS.
# This tool is provided without ANY warranty, guarantee and support
# It falls under the GPL v3.0 license
# You can use, abuse, modify or whatever you want with it.
# If you like the tool feel free to donate some beers
# BTC: 3KpA1zs7W6hd2YYqXFHX5cjyLTi8nBKpZN
# LTC: M9r2gXp6N1tvbbaKmQ5CVYBEAEzUsx9Nps
# Enjoy!

AMDGPU_COUNT=`/usr/bin/lspci -n -v |egrep -ic "0300: 1002"`
NVIDIA_COUNT=`/usr/bin/lspci -n -v |egrep -ic "0300: 10de|0302: 10de"`
TOTAL_COUNT=$( expr $NVIDIA_COUNT + $AMDGPU_COUNT )
API_PORT=4029
CLAYMORE_API=$(echo '{"id":0,"jsonrpc":"2.0","method":"miner_getstat1"}' | nc localhost $API_PORT)
TOTAL_HASH=$(echo "${CLAYMORE_API}"|jq -r '.result[2] | split(";")[0]|tonumber / 1000')
GPU_NUM=$*

Printer(){
        test -t 1 && echo -e "\033[1m $* \033[0m"
    }
## Kernel, Uptime, IP Address
KERNEL_VERSION=`uname -r`
UPTIME=`uptime -p`
ipAddress=`/sbin/ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`

## LOAD DATA TO JSON
function show_amd_stats() {
x=0
while [ $x -lt $1 ]; do
devid=$((x+1))
pci_id=$(lspci -n | grep 1002: | egrep -v "\.1" |awk '{print $1}' |sed -n ${devid}p)
                GPU_CORE=`cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/pp_dpm_sclk |grep "*" | awk -F  " " '{print $2}' | tr -d 'Mhz' | tr '\n' ' '`
                GPU_MEMORY=`cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/pp_dpm_mclk |grep "*" | awk -F  " " '{print $2}' | tr -d 'Mhz' | tr '\n' ' '`

                GPU_POWER=$(( $(echo `cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/power1_average`) / 1000000 ))
		GPU_MAX_POWER=$(( $(echo `cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/power1_cap`) / 1000000 ))
                GPU_TEMP1=$(( $(echo `cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/temp1_input`) / 1000 ))
                GPU_TEMP2=$(( $(echo `cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/temp2_input`) / 1000 ))

                GPU_FANSPEED=$(bc <<< "scale=2; (`cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/pwm1`/255)*100" | cut -d \. -f 1)

        if [ -f /sys/kernel/debug/dri/$x/amdgpu_pm_info ]; then
                GPU_VOLT=$(cat /sys/kernel/debug/dri/$x/amdgpu_pm_info |grep 'GPU Voltage' | awk '{print $1}'| sed 's/ //g')
	else
		GPU_VOLT=$(echo `cat /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/in0_input`)
        fi
Printer " [[ GPU$devid ==> | CoreClk: ${GPU_CORE}MHz | MemClk: ${GPU_MEMORY}MHz | Power Used: ${GPU_POWER}W | Power CAP: ${GPU_MAX_POWER}W | Voltage: ${GPU_VOLT}mV | Temp: ${GPU_TEMP1}C | ASIC Temp: ${GPU_TEMP2}C | Fanspeed: ${GPU_FANSPEED}% | ]]"

x=$((x+1))
        TOTAL_GPU_PWR="${TOTAL_GPU_PWR} ${GPU_POWER}"
done
        MEM_WATTS=$(($AMDGPU_COUNT * 47))
        TOTAL_WATTS=$(echo $TOTAL_GPU_PWR $MEM_WATTS 50| xargs  | sed -e 's/\ /+/g' | bc)
        MINER_EFFICIENCY=$(echo $TOTAL_HASH $TOTAL_WATTS | awk '{print $1 / $2}')

if [ $AMDGPU_COUNT -gt 0 ]; then
        Printer "\n\tCurrent hashrate: ${TOTAL_HASH} MH/s"
        Printer "\tCurrent AMD Total Powerdraw: ${TOTAL_WATTS}W"
        Printer "\tEfficiency of Claymore Dual ETH Miner: $(printf "%0.2f" ${MINER_EFFICIENCY}) MH per WATT"
fi
}


function show_nvi_stats() {
 x=0
 while [ $x -lt $1 ]; do
        NVI_GET_INFO=$(/usr/bin/nvidia-smi -i $x --query-gpu=clocks.gr,clocks.mem,power.draw,pstate,fan.speed,temperature.gpu --format=csv,noheader,nounits)
        GPU_CORE=$(echo $NVI_GET_INFO | awk -F', ' '{print $1}')
        GPU_MEM=$(echo $NVI_GET_INFO | awk -F', ' '{print $2}')
        GPU_POWER=$(echo $NVI_GET_INFO | awk -F', ' '{print $3}')
        GPU_PSTATE=$(echo $NVI_GET_INFO | awk -F', ' '{print $4}')
        GPU_TEMP=$(echo $NVI_GET_INFO | awk -F', ' '{print $6}')
        GPU_FAN=$(echo $NVI_GET_INFO | awk -F', ' '{print $5}')


Printer " [[ GPU$x ==> | GfxCLK: ${GPU_CORE}MHz | MemClk: ${GPU_MEM}MHz | Power: ${GPU_POWER}W | Power State: ${GPU_PSTATE} | Temp: ${GPU_TEMP}C | Fanspeed: ${GPU_FAN}% | ]]"
x=$((x+1))
        TOTAL_GPU_PWR="${TOTAL_GPU_PWR} ${GPU_POWER}"
done
        TOTAL_WATTS=$(echo $TOTAL_GPU_PWR 50| xargs  | sed -e 's/\ /+/g' | bc)
        MINER_EFFICIENCY=$(echo $TOTAL_HASH $TOTAL_WATTS | awk '{print $1 / $2}')
if [ $NVIDIA_COUNT -gt 0 ]; then
        Printer "\n\tCurrent hashrate: ${TOTAL_HASH} MH/s"
        Printer "\tCurrent NVIDIA Total Powerdraw: ${TOTAL_WATTS}W"
        Printer "\tEfficiency of Claymore Dual ETH Miner: $(printf "%0.2f" ${MINER_EFFICIENCY}) MH per WATT"
fi
}

function gpu_detect_amd() {
Printer "Input the GPU number you want to find:"
read -t 10 gpu_id
devid=$((gpu_id+1))
pci_id=$(lspci -n | grep 1002: | egrep -v "\.1" |awk '{print $1}' |sed -n ${devid}p)

if [ $AMDGPU_COUNT -gt 0 ]; then
        #killing miner
        screen -S miner -X quit
	Printer "\n[[ Found $AMDGPU_COUNT AMD GPUs in the system ]] "
	Printer "\n[[ Setting fan to 0% for GPU: $gpu_id ]] "
        Printer "\n[[ Command will repeat 5 times to make sure fan is totally stopped! ]] "
        Printer "\n[[ Make sure to remain eye contact on the Rig while spinning down fans! ]] "
	for ((n=0;n<5;n++))
	do
	echo 0 > /sys/devices/pci0000\:00/????:??:??.?/0000:${pci_id}/hwmon/hwmon?/pwm1
	sleep 0.5
	done
else
	Printer "No AMD Hardware found!"
	exit 0
fi
}

function gpu_detect_nvidia() {
Printer "Input the GPU number you want to find:"
read -t 10 devid
if [ $NVIDIA_COUNT -gt 0 ]; then
        #killing miner
	screen -S miner -X quit
        Printer "\n[[ Found $NVIDIA_COUNT NVIDIA GPUs in the system ]] "
        Printer "\n[[ Setting fan to 0% on GPU: $devid ]] "
        Printer "\n[[ Command will repeat 2 times to make sure fan is totally stopped! ]] "
        Printer "\n[[ Make sure to remain eye contact on the Rig while spinning down fans! ]] "
        for ((n=0;n<2;n++))
        do
		DISPLAY=:0.$devid /usr/bin/nvidia-settings -a [gpu:$devid]/GPUFanControlState=1 -a [fan:$devid]/GPUTargetFanSpeed=0
		sleep 0.2
        done
else
        echo -e "No NVIDIA Hardware found!"
        exit 0
fi
}

function show_help() {
        echo -e "Usage: '--efficiency','--detect *gpunum*'"
}

case "$1" in
	--detect-amd) gpu_detect_amd;;
	--detect-nv) gpu_detect_nvidia;;
        --help|-h) show_help;;
        --efficiency|*)
show_amd_stats $AMDGPU_COUNT
show_nvi_stats $NVIDIA_COUNT
Printer "\n\tShowing total of $TOTAL_COUNT GPU information"
Printer "\tOf which $AMDGPU_COUNT are AMD and $NVIDIA_COUNT are NVIDIA"
Printer "\n\tRig Kernel: $KERNEL_VERSION,\t Rig Uptime: $UPTIME,\t IP:  $ipAddress"
;;
esac
exit 0
