#!/bin/bash
cd "$(dirname "$0")"
mkimage -A mips -O linux -T script -C none -a 0 -e 0 -n "OpenIPC SD Update" -d boot.cmd boot.scr
