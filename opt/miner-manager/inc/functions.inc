#!/bin/bash
source /opt/miner-manager/inc/globals.inc

function plainlogger() {
        test -t 1 && echo -e "$*"
}

function infologger() {
        test -t 1 && echo -e "`date +%Y.%m.%d-%H:%M:%S` $*"
}

function loop_nviget_model() {
        x=0
        while [ $x -lt $1 ]; do
        nvi_model=`/usr/bin/nvidia-smi -i $x --query-gpu=gpu_name --format=csv,noheader`
        infologger "nVidia Model GPU${x}: $nvi_model"
        x=$((x+1))
        done
}
function loop_amdget_model() {
        x=0
        while [ $x -lt $1 ]; do
        allamdgpus=`lspci -nnv | egrep "VGA|Display" | grep "1002" | cut -d"." -f1,3   | sed 's#\[AMD/ATI\]##g' | sed 's/:00. //g' | sed 's/Radeon //g' | sed 's/OEM //g'| sed -e 's/\[1002:....]\s*//g' | \cut -d"(" -f1 | awk '{print "GPU" i++ ": " $0}' > /tmp/amdgpu.outfile`
        amd_model=`cat /tmp/amdgpu.outfile |grep GPU${x}`
        setuplogger "AMD Model $amd_model"
        x=$((x+1))
        done
}

function print_gpu_profiles() {
 if [ $nvi_count -gt 0 ]; then
	nvprof_count=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | .prof_id' |wc -l)
	infologger "\033[1mAvailable nVidia profiles ->\033[0m"
     x=0
     while [ $x -lt $nvprof_count ]; do
        nvprof_available=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$x') |  "\(.prof_id) \(.name)"' |sed -e 's/{//g' -e 's/}//g')
	plainlogger "\033[1m$nvprof_available  \033[0m"
     x=$((x+1))
     done
 elif [ $amd_count -gt 0 ]; then
        amdprof_count=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] | .prof_id' |wc -l)
	infologger "\033[1mAvailable AMD profiles ->\033[0m"
     x=0
     while [ $x -lt $amdprof_count ]; do
        amdprof_available=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] |select('.prof_id' == '$x') |  "\(.prof_id) \(.name)"' |sed -e 's/{//g' -e 's/}//g')
        plainlogger "\033[1m$amdprof_available  \033[0m"
     x=$((x+1))
     done
 else
	infologger "No GPU profiles available for this Algo"
 fi
}

function print_miner_profiles() {
 if [ -f $mOScfg/miner-profiles.json ]; then
	 mprof_count=$(echo `cat $mOScfg/miner-profiles.json` | jq -r '.minerSettings[] | .miner_id' |wc -l)
         infologger "\033[1mAvailable miners -> \033[0m"
     x=0
     while [ $x -lt $mprof_count ]; do
             miners_available=$(echo `cat $mOScfg/miner-profiles.json` | jq -r '.minerSettings[] | select('.miner_id' == '$x') |  "\(.miner_id) \(.miner_name)"' |sed -e 's/{//g' -e 's/}//g')
	     plainlogger "\033[1m$miners_available  \033[0m"
     x=$((x+1))
     done
 else
	 infologger "No available config found!"
 fi
}

function gen_nvi_section() {
       if [ -z ${nvidia_profile+set} ]; then
        nvidia_prof_name="Nonexisting"
        nvidia_coreoc="0"
        nvidia_memoc="0"
        nvidia_powlim="0"
        nvidia_powthd="50"
        setuplogger "No nvidia profile set defaults are Name: $nvidia_prof_name, CoreOC: $nvidia_coreoc, MemOC: $nvidia_memoc, Powerlimit: $nvidia_powlim, Power threashod: $nvidia_powthd"
       else
	nvidia_prof_name=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$nvidia_profile')| .name')
        nvidia_coreoc=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$nvidia_profile')| .coreoc')
        nvidia_memoc=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$nvidia_profile')| .memoc')
        nvidia_powlim=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$nvidia_profile')| .powlim')
        nvidia_powthd=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.nvidia_profiles[] | select('.prof_id' == '$nvidia_profile')| .powthd')
	setuplogger "Profile set to: [ $nvidia_prof_name ]! Your Core is clocked with: [ ${nvidia_coreoc}Mhz ], your Memory is clocked with: [ ${nvidia_memoc}Mhz ], Powerlimit set to: [ ${nvidia_powlim}W ]"
       fi
       echo -e "{\n\t\t\"nvidia\"":"\t"["\n\t"{"\n\t \"gpuSettings"\":"\n\t\t "["\n\t\t" >> $mOScfg/gpu-config.json
        x=0
        while [ $x -lt $1 ]; do
        if [ $x -lt $(expr $1 - 1) ] ; then
        echo -e "{\n\t\t\"gpu_id"\":" "$x",\n\t\t \"Core"\":" "$nvidia_coreoc",\n\t\t \"Mem"\":" "$nvidia_memoc",\n\t\t \"PowLim"\":" "$nvidia_powlim",\n\t\t \"PowTHD"\":" "$nvidia_powthd",\n\t\t \"MinFan"\":" "$minfan",\n\t\t \"TempTarget"\":" "$temptarget"\n\t "},"" >> $mOScfg/gpu-config.json
        else
        echo -e "{\n\t\t\"gpu_id"\":" "$x",\n\t\t \"Core"\":" "$nvidia_coreoc",\n\t\t \"Mem"\":" "$nvidia_memoc",\n\t\t \"PowLim"\":" "$nvidia_powlim",\n\t\t \"PowTHD"\":" "$nvidia_powthd",\n\t\t \"MinFan"\":" "$minfan",\n\t\t \"TempTarget"\":" "$temptarget"\n\t "}"" >> $mOScfg/gpu-config.json
        fi
        x=$((x+1))
        done
        echo -e "\n\t "]"\n\t"}"\n\t"]",\n" >> $mOScfg/gpu-config.json
	sleep 0.2
}
function gen_amd_section() {
       if [ -z ${amdgpu_profile+set} ]; then
        amdgpu_prof_name="Nonexisting"
        amdgpu_coreoc="0"
        amdgpu_memoc="0"
        amdgpu_powtune="0"
        amdgpu_powthd="50"
        setuplogger "No amdgpu profile set"
        setuplogger "No amdgpu profile set defaults are Name: $amdgpu_prof_name, CoreOC: $amdgpu_coreoc, MemOC: $amdgpu_memoc, Powerlimit: $amdgpu_powtune, Power threashod: $amdgpu_powthd"
	else
        amdgpu_prof_name=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] |  select('.prof_id' == '$amdgpu_profile')| .name')
        amdgpu_coreoc=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] | select('.prof_id' == '$amdgpu_profile')| .coreoc')
        amdgpu_memoc=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] |  select('.prof_id' == '$amdgpu_profile')| .memoc')
        amdgpu_powtune=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] |  select('.prof_id' == '$amdgpu_profile')| .powtune')
        amdgpu_powthd=$(echo `cat $mOScfg/gpu-profiles.json` | jq -r '.amdgpu_profiles[] |  select('.prof_id' == '$amdgpu_profile')| .powthd')
        setuplogger "Profile set to: [ $amdgpu_prof_name ]! Your Core is set to: [ ${amdgpu_coreoc}Mhz ], your Memory is set to: [ ${amdgpu_memoc}Mhz ], Core DPM Powerstate is set to: [ ${amdgpu_powtune} ]"
        fi
        echo -e "\"amdgpu\"":"\t"["\n\t"{"\n\t \"gpuSettings"\":"\n\t\t "["\n\t\t" >> $mOScfg/gpu-config.json
        x=0
        while [ $x -lt $1 ]; do
        if [ $x -lt $(expr $1 - 1) ] ; then
        printf "{\n\t\t\"gpu_id"\":" "$x",\n\t\t \"Core"\":" "$amdgpu_coreoc",\n\t\t \"Mem"\":" "$amdgpu_memoc",\n\t\t \"PTune"\":" "$amdgpu_powtune",\n\t\t \"PowTHD"\":" "$amdgpu_powthd",\n\t\t \"gpu_volt"\":" "0",\n\t\t\"MinFan"\":" "$minfan",\n\t\t \"TempTarget"\":" "$temptarget"\n\t"}",\n\t\t" >> $mOScfg/gpu-config.json
        else
        printf "{\n\t\t\"gpu_id"\":" "$x",\n\t\t \"Core"\":" "$amdgpu_coreoc",\n\t\t \"Mem"\":" "$amdgpu_memoc",\n\t\t \"PTune"\":" "$amdgpu_powtune",\n\t\t \"PowTHD"\":" "$amdgpu_powthd",\n\t\t \"gpu_volt"\":" "0",\n\t\t\"MinFan"\":" "$minfan",\n\t\t \"TempTarget"\":" "$temptarget"\n\t"}"\n\t\t" >> $mOScfg/gpu-config.json
        fi
        x=$((x+1))
        done
        echo -e "\n\t "]"\n\t"}"\n"]"\n"}"" >> $mOScfg/gpu-config.json
	sleep 0.2
}

function miner_start() {
if ! pgrep screen >/dev/null 2>&1
	then
		miner_id=$1
		miner_name=$(echo `cat $mOScfg/miner-profiles.json` | jq -r '.minerSettings[] | select('.miner_id' == '$miner_id')| .miner_name')
		miner_bin=$(echo `cat $mOScfg/miner-profiles.json` | jq -r '.minerSettings[] | select('.miner_id' == '$miner_id')| .miner_bin')
		miner_cmd=$(echo `cat $mOScfg/miner-profiles.json` | jq -r '.minerSettings[] | select('.miner_id' == '$miner_id')| .miner_cmd')
			if [ ! -z ${miner_id+set} ] && [ ! -z ${miner_name+set} ] && [ ! -z ${miner_bin+set} ] && [ ! -z ${miner_cmd+set} ]; then
				export GPU_MAX_HEAP_SIZE=100
				export GPU_USE_SYNC_OBJECTS=1
				export GPU_MAX_ALLOC_PERCENT=100
				screen -dmS miner $miner_bin $miner_cmd
				setuplogger "Miner with ID: $miner_id Started\n"
				setuplogger "Miner name: $miner_name\n"
				setuplogger "Startup command: $miner_cmd\n"
			else
			setuplogger "Something went wrong!"
			fi
else
	infologger "Miner process detected. Use minermanager --stop to stop it first!"
	exit 0
fi
}

function miner_stop() {
screen -S miner -X quit
setuplogger "Miner stopped"
}

function nvi_hw_powerinfo() {
x=0
 while [ $x -lt $1 ]; do
        infologger "Checking for dead nVidia GPUs..."
        POWER_THREASHOLD=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .PowTHD')
        power_draw=$(/usr/bin/nvidia-smi --id=$x --query-gpu=power.draw --format=csv,noheader,nounits)
        sleep 0.1
 if (( $(echo "$power_draw < $POWER_THREASHOLD" | bc -l) ))
        then
           infologger  "Power dropped below threshold (${POWER_THREASHOLD} W)!, nVidia GPU $x: $power_draw W, Activating watchdog cycle!"
           activate_watchdog_cycle
   else
           infologger "Normal power usage for nVidia GPU $x: $power_draw W"
   fi
 x=$((x+1))
 done
}

function amd_hw_powerinfo() {
x=0
 while [ $x -lt $1 ]; do
        infologger "Checking for dead AMD GPUs..."
        POWER_THREASHOLD=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .PowTHD')
        power_draw=$(cat /sys/kernel/debug/dri/$x/amdgpu_pm_info |grep '(average GPU)'|awk '{print $1}'| sed 's/ //g')
        sleep 0.1
 if (( $(echo "$power_draw < $POWER_THREASHOLD" | bc -l) ))
        then
           infologger  "Power dropped below threshold (${POWER_THREASHOLD} W)!, AMD GPU $x: $power_draw W, Activating watchdog cycle!"
           activate_watchdog_cycle
   else
           infologger "Normal power usage for AMD GPU $x: $power_draw W"
   fi
 x=$((x+1))
 done
}

function activate_watchdog_cycle() {
CHECK_SCREEN=$(screen -list |egrep -ic "miner")
MINER_OUT=$(screen -x miner -X hardcopy /tmp/miner| cat /tmp/miner)
MINERCRASH=$(echo "${MINER_OUT}"| egrep -ic "crashed \|Error communicating")
OCLCRASH=$(echo "${MINER_OUT}"|egrep -ic "hangs in OpenCL call, exit") # Claymore
CUDACRASH=$(echo "${MINER_OUT}"|egrep -ic "CUDA error\|error CUDA\|an illegal memory access was encountered\|error exception") # CUDA & OpenCL ethminer
NVCRASH=$(dmesg |egrep -ic "NVRM: Xid") # HW Error
if [ $CHECK_SCREEN = 1 ]; then
        infologger "Screen is running, checking other stuff"
    if [ $CUDACRASH -gt 0 ] || [ $OCLCRASH -gt 0 ] || [ $NVCRASH -gt 0 ]; then
            infologger "GPU crashed, activating watchdog hard reset!"
            $mOSbin/force_reboot.sh
    elif [ $MINERCRASH -gt 0 ]; then
            infologger "Miner crashed restarting it..."
            on_watchdog_notify
            $mOSbin/minermanager --stop
        sleep 1
            $mOSbin/minermanager --start
    else
        infologger "No GPU error found, sending Telegram just in case"
        unknown_error_notify
    fi

elif [ $CHECK_SCREEN = 0 ]; then
        infologger "Screen is not running and no GPU error found...starting"
        on_watchdog_notify
        $mOSbin/minermanager --start
else
        infologger "Something else is wrong..sending notifications!"
        unknown_error_notify
fi

}

function nvi_powerlimit() {
 x=0
 while [ $x -lt $1 ]; do
        POWERLIMIT=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .PowLim')
        minPLIMIT=`/usr/bin/nvidia-smi -i $x -q -d POWER |grep "Min Power Limit" |awk '{print $5}' | cut -d"." -f1  |xargs`
		infologger "Supported minimal PowerLimit for GPU: $x is $minPLIMIT!"
        maxPLIMIT=`/usr/bin/nvidia-smi -i $x -q -d POWER |grep "Max Power Limit" |awk '{print $5}' | cut -d"." -f1  |xargs`
                infologger "Supported maximum PowerLimit for GPU: $x is $maxPLIMIT!"
	  if [ $POWERLIMIT -ge $minPLIMIT ] && [ $POWERLIMIT -le $maxPLIMIT ]; then
        	/usr/bin/nvidia-smi -i $x -pm 0 >/dev/null
	        /usr/bin/nvidia-smi -i $x -pl $POWERLIMIT >/dev/null
	  infologger "PowerLimit was set to $POWERLIMIT for GPU: $x"
	  else
	  infologger "Powerlimit You are setting is not supported on GPU: $x !"
	  infologger "Please use values greater than $minPLIMIT and less than $maxPLIMIT!"
	  fi

  x=$((x+1))
  done
}

function amd_corepower_clock() {
 x=0
 while [ $x -lt $1 ]; do
	POWERTUNE=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .PowTune')
	CORECLK=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .Core')
     if [ "$POWERTUNE" -gt "0" ] && [ "$POWERTUNE" -le "7" ]  && [ "$CORECLK" -gt "0" ]; then
        infologger "Setting PowerTune for AMD GPU${x} to dpm state $POWERTUNE"
        echo "manual" > /sys/class/drm/card$x/device/power_dpm_force_performance_level
        echo "$POWERTUNE" > /sys/class/drm/card$x/device/pp_dpm_sclk >/dev/null
        for i in $(seq $POWERTUNE 7); do
        /usr/local/bin/ohgodatool -i $x --core-state $i --core-clock $CORECLK
        done
        infologger "Setting Core dpm clock for AMD GPU${x} to $CORECLK"
     else
        infologger "Powertune and coreclock are undefined, leaving default..."
     fi
x=$((x+1))
done
}

function amd_memory_clock() {
 x=0
 while [ $x -lt $1 ]; do
	MEMCLK=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .Mem')
	MEMSTATES=$(/usr/local/bin/ohgodatool -i $x | egrep -ic "Memory state")
	if [ $MEMCLK -gt 0 ]; then
	     if [ "$MEMSTATES" -gt 2 ] && [ $MEMCLK -gt 300 ] && [ $MEMCLK -le 2300 ]; then
	        /usr/local/bin/ohgodatool -i $x --mem-state 2 --mem-clock $MEMCLK >/dev/null
	        infologger "Setting memory on AMD GPU: $x to $MEMCLK"
	     elif [ $MEMSTATES -le 2 ] && [ $MEMCLK -gt 300 ] && [ $MEMCLK -le 2300 ]; then
	        /usr/local/bin/ohgodatool -i $x --mem-state 1 --mem-clock $MEMCLK >/dev/null
	        infologger "Setting memory on AMD GPU: $x to $MEMCLK"
	     else
	        infologger "Wrong values?"
	     fi
	else
           infologger "No Memory clock defined for GPU: $x"
        fi
  x=$((x+1))
  done
}

function nvi_core_clock() {
 x=0
 while [ $x -lt $1 ]; do
        CORE=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .Core')
		if [ $CORE -ge "-200" ] && [ $CORE -le "250" ]; then
		        DISPLAY=:0.$x /usr/bin/nvidia-settings -a [gpu:$x]/GPUPowerMizerMode=1 >/dev/null
		        DISPLAY=:0.$x /usr/bin/nvidia-settings -a [gpu:$x]/GPUGraphicsClockOffsetAllPerformanceLevels=$CORE >/dev/null
		        infologger "Setting Gfx clock to $CORE offset for GPU: $x"
		else
			infologger "No Gfx clock defined for GPU: $x"
		fi
  x=$((x+1))
  done
}

function nvi_memory_clock() {
 x=0
 while [ $x -lt $1 ]; do
        MEMORY=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .Mem')
                if [ $MEMORY -ge "-400" ] && [ $MEMORY -le "2000" ]; then
		        DISPLAY=:0.$x /usr/bin/nvidia-settings -a [gpu:$x]/GPUPowerMizerMode=1 >/dev/null
		        DISPLAY=:0.$x /usr/bin/nvidia-settings -a [gpu:$x]/GPUMemoryTransferRateOffsetAllPerformanceLevels=$MEMORY >/dev/null
		        infologger "Setting Memory clock to $MEMORY offset for GPU: $x"
		                else
                        infologger "No Memory clock defined for GPU: $x"
                fi
  x=$((x+1))
  done
}

#function amd_set_voltage() {
# x=0
# while [ $x -lt $1 ]; do
#        VOLTAGE=$(echo `cat $mOScfg/gpu-config.json` | jq -r '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .gpu_volt')
#        if [ "$VOLTAGE" = 0 ]; then
#                infologger "No Voltage defined for GPU: $x"
#	else
#        	$mOSbin/amdvoltset --index $x --voltage $VOLTAGE >/dev/null
#	        infologger "Voltage set to $VOLTAGE for GPU: $x"
#        fi
# x=$((x+1))
# done
#}

function decide_fanspeed() {
TT=$1
FS=$2
RT=$3
RF=$4
# Credits to tytanick for this fan descission script -> http://simplemining.net
	    FAN_UP_STEP=30
	    FAN_DOWN_STEP=2

	DIFF=`echo "$((TT-RF))" | sed 's/-//g'`

	if [ $DIFF -ge 7 ]; then
	    FAN_UP_STEP=100
	    FAN_DOWN_STEP=2
	elif [ $DIFF -ge 5 ]; then
	    FAN_UP_STEP=30
	    FAN_DOWN_STEP=1
	elif [ $DIFF -ge 4 ]; then
	    FAN_UP_STEP=12
	    FAN_DOWN_STEP=1
	elif [ $DIFF -ge 3 ]; then
	    FAN_UP_STEP=8
	    FAN_DOWN_STEP=1
	elif [ $DIFF -ge 2 ]; then
	    FAN_UP_STEP=1
	    FAN_DOWN_STEP=1
	elif [ $DIFF -ge 1 ]; then
	    FAN_UP_STEP=1
	    FAN_DOWN_STEP=0
	elif [ $DIFF -ge 0 ]; then
	    FAN_UP_STEP=0
	    FAN_DOWN_STEP=0

	fi

	NEW_GPU_FAN=$RF

	    if [ $RT -gt $((TT)) ]; then
	        NEW_GPU_FAN=$(( GPU_FAN + FAN_UP_STEP ))
	    fi

	    if [ $RT -lt $((TT-2)) ]; then
	        NEW_GPU_FAN=$(( GPU_FAN - FAN_DOWN_STEP ))
	    fi


	if [ $NEW_GPU_FAN -le $FS ]; then
	    NEW_GPU_FAN=$FS
	fi

	if [ $NEW_GPU_FAN -ge 100 ]; then
	    NEW_GPU_FAN=100
	fi
	echo "$NEW_GPU_FAN"
}

function set_amd_fanspeed() {
x=0
while [ $x -lt $1 ]; do
        TempTarget=$(echo `cat $mOScfg/gpu-config.json` | jq '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .TempTarget')
        FanSpeed=$(echo `cat $mOScfg/gpu-config.json` | jq '.amdgpu[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .MinFan')
        RealTemp=$(/usr/local/bin/ohgodatool -i $x  --show-temp | tr -d 'C' )
        RealFan=$(/usr/local/bin/ohgodatool -i $x --show-fanspeed | tr -d '%' )
	NewFan=$(decide_fanspeed $TempTarget $FanSpeed $RealTemp $RealFan)
	/usr/local/bin/ohgodatool -i $x --set-fanspeed $NewFan >/dev/null
	infologger "Setting Fanspeed to $NewFan% for GPU: $x"
 x=$((x+1))
 done
}

function set_nvi_fanspeed() {
x=0
while [ $x -lt $1 ]; do
        TempTarget=$(echo `cat $mOScfg/gpu-config.json` | jq '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .TempTarget')
        FanSpeed=$(echo `cat $mOScfg/gpu-config.json` | jq '.nvidia[] | .gpuSettings[] |  select('.gpu_id' == '$x')| .MinFan')
        RealTemp=$(/usr/bin/nvidia-smi -i $x -q -d TEMPERATURE | grep "Current" | sed 's/[^0-9]*//g')
        RealFan=$(/usr/bin/nvidia-smi -i $x -q |grep -i fan | sed 's/[^0-9]*//g')
        NewFan=$(decide_fanspeed $TempTarget $FanSpeed $RealTemp $RealFan)
        DISPLAY=:0.$x /usr/bin/nvidia-settings -a [gpu:$x]/GPUFanControlState=1 -a [fan:$x]/GPUTargetFanSpeed=$NewFan >/dev/null
        infologger "Setting Fanspeed to $NewFan % for GPU: $x"
x=$((x+1))
done
}

function get_nvi_stats() {
 x=0
 while [ $x -lt $1 ]; do
        statgclk=$(/usr/bin/nvidia-smi -i $x --query-gpu=clocks.gr --format=csv |tail -n +2)
        statmclk=$(/usr/bin/nvidia-smi -i $x --query-gpu=clocks.mem --format=csv |tail -n +2)
        statpwr=$(/usr/bin/nvidia-smi -i $x --query-gpu=power.draw --format=csv |tail -n +2)
        statpst=$(/usr/bin/nvidia-smi -i $x --query-gpu=pstate --format=csv |tail -n +2)
        statfan=$(/usr/bin/nvidia-smi -i $x --query-gpu=fan.speed --format=csv |tail -n +2)
        stattemp=$(/usr/bin/nvidia-smi -i $x --query-gpu=temperature.gpu --format=csv |tail -n +2)
        infologger "Current nVidia GPU: $x Core Clock is: \033[1m$statgclk\033[0m"
        infologger "Current nVidia GPU: $x Memory Clock is: \033[1m$statmclk\033[0m"
        infologger "Current nVidia GPU: $x Power Usage is: \033[1m$statpwr\033[0m"
        infologger "Current nVidia GPU: $x Power State is: \033[1m$statpst\033[0m"
        infologger "Current nVidia GPU: $x Fan Speed is: \033[1m$statfan\033[0m"
        infologger "Current nVidia GPU: $x Temperature is: \033[1m$stattemp\033[0m\n"
 x=$((x+1))
 done
}

function get_amd_stats() {
 x=0
 while [ $x -lt $1 ]; do
        if [ -f /sys/class/drm/card$x/device/pp_table ]; then
                statgclk=$(cat /sys/class/drm/card$x/device/pp_dpm_sclk |grep "*" | awk -F  " " '{print $2}' | tr -d 'Mhz' | tr '\n' ' ')
                statmclk=$(cat /sys/class/drm/card$x/device/pp_dpm_mclk |grep "*" | awk -F  " " '{print $2}' | tr -d 'Mhz' | tr '\n' ' ')
        fi
        if [ -f /sys/kernel/debug/dri/$x/amdgpu_pm_info ]; then
                statpwr=$(cat /sys/kernel/debug/dri/$x/amdgpu_pm_info |grep '(average GPU)'|awk '{print $1}'| sed 's/ //g')
                stattemp=$(cat /sys/kernel/debug/dri/$x/amdgpu_pm_info |grep 'GPU Temperature'|awk '{print $3}'| sed 's/ //g')
        fi
        if [ -f /sys/class/drm/card$x/device/pp_table ]; then
                statfan=$(/usr/local/bin/ohgodatool -i $x --show-fanspeed)
        fi
        infologger "Current AMD GPU: $x Core Clock is: \033[1m${statgclk}Mhz\033[0m"
        infologger "Current AMD GPU: $x Memory Clock is: \033[1m${statmclk}Mhz\033[0m"
        infologger "Current AMD GPU: $x Power Usage is: \033[1m${statpwr}W\033[0m"
        infologger "Current AMD GPU: $x Fan Speed is: \033[1m$statfan\033[0m"
        infologger "Current AMD GPU: $x Temperature is: \033[1m$stattemp\033[0m\n"
 x=$((x+1))
 done
}

function total_gpu_list() {
	GPULIST=$(lspci -nnv | egrep "\[0300\]:|\[0302\]:" | cut -d " " -f 1,5,7- | cut -d "(" -f 1 | cut -d "[" -f 1,3 | sed -e 's/\[//g' -e 's/\]//g' -e "s/:00.0//g")
	printf "\033[1m$GPULIST\033[0m\n"
}


function show_help() {
        echo -e "Usage: '--start','--stop' to start or stop miner"
        echo -e "Usage: '--monitor' to view current mining stats and '--powerinfo' to check power usage"
        echo -e "Usage: '--setclocks', '--setfanspeed' to apply settings per GPU defined in conf.json"
        echo -e "Usage: '--status', '--list' for current GPU stats and complete list"
        echo -e "Usage: '--notify', \033[1mNOT IN FREE VERSION\033[0m"
        echo -e "Usage: '--efficiency', \033[1mNOT IN FREE VERSION\033[0m"
        echo -e "Usage: '--version', to see current minermanager version"
        echo -e "Usage: '--setvoltage' \033[1mNOT IN FREE VERSION\033[0m"
        echo -e "Donation addresses"
        echo -e "Zec: \033[1mt1WMvBRutsSCW3c4p6uFvx6SP7nQDJPBdCc\033[0m"
        echo -e "Ether: \033[1m0xE1D39706a292a349D719E9f63dB04eB0a6E09c9D\033[0m"
        echo -e "BTC: \033[1m3KpA1zs7W6hd2YYqXFHX5cjyLTi8nBKpZN\033[0m"
        echo -e "LTC: \033[1mM9r2gXp6N1tvbbaKmQ5CVYBEAEzUsx9Nps\033[0m"
        echo -e "Enjoy!"
        exit 0
}

function show_version() {
	echo -e "Release Ver: \033[1m$version\033[0m"
	echo -e "Use --help or -h for more info"
	exit 0
}
