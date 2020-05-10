#!/bin/bash
if [ -z $1 ]; then
	echo "Missing argument..."
	echo "Penggunaan:"
	echo "   ./test <topic> <message>"
	echo "Note: kirim topik general/text dan pesan stop untuk berhenti"
	exit -1
fi
mosquitto_pub -h 127.0.0.1 -t "$1" -m "$2"
