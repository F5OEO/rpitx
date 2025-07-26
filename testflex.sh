#!/bin/sh

printf "$2" | sudo ./flex -f "$1"
