#!/bin/bash

bluetoothctl --timeout 10 scan on > /dev/null
devices=$(bluetoothctl devices)
conndevices=$(bluetoothctl devices Connected)

while IFS= read -r line || [[ -n "$line" ]]; do
	dev_uuid=${line:7:17}
	dev_name=${line:25}

	is_connected=0
	while IFS= read -r conn_line || [[ -n "$conn_line" ]]; do
		conn_dev_uuid=${conn_line:7:17}
		conn_dev_name=${conn_line:25}
		if [[ "$dev_uuid" == "$conn_dev_uuid" ]]; then
			is_connected=1
		fi
	done <<< "$conndevices"	
	
    echo ${dev_uuid}$'\t'${dev_name}$'\t'${is_connected}
done <<< "$devices"
