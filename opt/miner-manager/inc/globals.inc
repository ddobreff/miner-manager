#!/bin/bash
version="0.5rc1"
mOScfg='/opt/miner-manager/etc'
mOSbin='/opt/miner-manager/bin'
# Getting GPU count per vendor
nvi_count=$(/usr/bin/lspci -n -v |egrep -ic "0300: 10de|0302: 10de")
amd_count=$(/usr/bin/lspci -n -v |egrep -ic "0300: 1002")
# Network interface and IP detection
iiface=$(/sbin/ip -o link show | awk '{print $2,$9}' | grep "UP" | sed 's/://g' | awk '{print $1}')
ipAddress=$(/sbin/ifconfig $iiface | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')
# Getting Total GPU count
totalgpus=$( expr $nvi_count + $amd_count )
# Miner ID and profiles
miner_id=$(echo `cat $mOScfg/global-config.json` | jq -r '.globalConfig[] | select('.active_config' == '1')| .miner_profile')
minfan=$(echo `cat $mOScfg/global-config.json` | jq -r '.globalConfig[] | select('.miner_profile' == '$miner_id')| .minfan')
temptarget=$(echo `cat $mOScfg/global-config.json` | jq -r '.globalConfig[] | select('.miner_profile' == '$miner_id')| .temptarget')
nvidia_profile=$(echo `cat $mOScfg/global-config.json` | jq -r '.globalConfig[] | select('.miner_profile' == '$miner_id')| .nvidia_profile_id')
amdgpu_profile=$(echo `cat $mOScfg/global-config.json` | jq -r '.globalConfig[] | select('.miner_profile' == '$miner_id')| .amdgpu_profile_id')
