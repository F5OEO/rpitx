#!/bin/sh

printf "$2" | sudo ./pocsag -f "$1"
