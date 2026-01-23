#!/bin/bash
nmcli dev wifi rescan

nmcliout=$(nmcli dev wifi list)
nmclihead=$(echo "$nmcliout" | head -n 1)
bssidindex=$(( $(echo "$nmclihead" | awk '{print index($0, " BSSID")}') + 1 ))
ssidindex=$(( $(echo "$nmclihead" | awk '{print index($0, " SSID")}') + 1 ))
modeindex=$(( $(echo "$nmclihead" | awk '{print index($0, " MODE")}') + 1 ))
barsindex=$(( $(echo "$nmclihead" | awk '{print index($0, " BARS")}') + 1 ))
securityindex=$(( $(echo "$nmclihead" | awk '{print index($0, " SECURITY")}') + 1 ))
nmclilist=$(echo "$nmcliout" | tail -n +2)

bssidlist=$(echo "$nmclilist" | awk '{ print substr($0, '$bssidindex', '$(($ssidindex - $bssidindex))')}')
readarray -t bssidarray <<< "$bssidlist"

ssidlist=$(echo "$nmclilist" | awk '{ print substr($0, '$ssidindex', '$(($modeindex - $ssidindex))')"("substr($0, '$barsindex', '$(($securityindex - $barsindex))')")"}')
readarray -t ssidarray <<< "$ssidlist"

selectedlist=$(echo "$nmclilist" | awk '{print substr($0,1,1)}' | sed 's/ /0/g' | sed 's/*/1/g')
readarray -t selectedarray <<< "$selectedlist"

for i in "${!bssidarray[@]}"; do
    printf "%s\t%s\t%s\n" "${bssidarray[i]}" "${ssidarray[i]}" "${selectedarray[i]}"
done

