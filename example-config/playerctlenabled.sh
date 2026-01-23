#!/bin/bash

# if playerctl paused more than ${VALID_TIME} secodnds, this script will output "0"
VALID_TIME=10
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEMP_FILE="$SCRIPT_DIR/~playerctlstatus.txt"

status_arr=("0","0","0")

if [ -f "$TEMP_FILE" ]; then
	status_arr=()
	while IFS= read -r line; do
		status_arr+=("$line")
	done < "$TEMP_FILE"
fi

CURRENTLY_PLAYING="$(playerctl metadata 2>&1 | grep -oP 'xesam:title.*' | awk -v n=2 '{ for (i=n; i<=NF; i++) printf "%s%s", $i, (i<NF ? OFS : ORS)}')"
IS_PLAYER_EXIST="$([[ $(playerctl status 2>&1) == *"No players found"* ]] && echo 0 || echo 1)"
IS_PAUSED="$([[ $(playerctl status 2>&1) == *"Paused"* ]] && echo 1 || echo 0)"

if [[ $IS_PLAYER_EXIST -eq 1 ]]; then
	if [[ $IS_PAUSED -eq 1 ]]; then
		if [[ "${status_arr[0]}" == "$CURRENTLY_PLAYING" && ${status_arr[1]} -eq 1 ]]; then
			LAST_PAUSE_AGE=$(( $(date +%s) - ${status_arr[2]} ))
			if [[ $LAST_PAUSE_AGE -gt 10 ]]; then
				echo "0"
			else 
				echo "1"
			fi
		else
			echo "1"
		fi
	else
		echo "1"
	fi
else
	echo "0"
fi

if [[ "$CURRENTLY_PLAYING" != "${status_arr[0]}" || "$IS_PAUSED" != "${status_arr[1]}" ]]; then
	echo ${CURRENTLY_PLAYING}$'\n'${IS_PAUSED}$'\n'$(date +%s)$'\n' > ${TEMP_FILE}
fi
