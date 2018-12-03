#!/bin/sh

printf "1:YOURCALL\n2: Hello world" | sudo ./pocsag -f "$1"
