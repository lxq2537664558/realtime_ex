#!/bin/sh
cd ../bin/linux
read kill_pid < $1.pid
kill -10 $kill_pid
