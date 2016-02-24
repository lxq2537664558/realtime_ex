#!/bin/sh
cd ../bin_linux/Debug
read ShellGate_pid < ShellGate.pid
kill -10 $ShellGate_pid
read ShellGas_pid < ShellGas.pid
kill -10 $ShellGas_pid
read ShellGcc_pid < ShellGcc.pid
kill -10 $ShellGcc_pid
