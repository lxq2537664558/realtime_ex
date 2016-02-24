#!/bin/sh
cd ../bin_linux/Debug
read kill_pid < $1.pid
kill -10 $kill_pid
