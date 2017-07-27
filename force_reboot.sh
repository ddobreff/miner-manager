#!/bin/bash
date=$(date)
HOSTNAME=`hostname -f`
## Telegram variables ##
chat_id="99999999" # Edit this to match your Telegram chat id
bot_id="999999" # Edit this to match your Telegram bot id
api_key="12r0f9jdfashfgasdfasdh102fasfas" # Edit this to match your Telegram room API key
## End telegram variables ##
ERROR=`screen -x ethminer -X hardcopy /tmp/miner && cat /tmp/miner | grep . | tail -n 30`

VALUE="*$HOSTNAME* Date: $date *CONSOLE OUTPUT*: *$ERROR*"

JSON="{\"chat_id\":$tchat_id,\"text\":\"$VALUE\",\"disable_notification\":true,\"parse_mode\":\"Markdown\"}"
curl -g -H "Content-Type: application/json" -X POST https://api.telegram.org/bot$bot_id:$api_key/sendMessage -d "$JSON"
sleep 3
sync
echo 1 > /proc/sys/kernel/sysrq
echo b > /proc/sysrq-trigger
