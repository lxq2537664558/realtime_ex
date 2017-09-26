#!/bin/sh
cd ../bin/linux
chmod 777 launcher
valgrind --tool=memcheck --undef-value-errors=yes --leak-check=full --error-limit=no --show-reachable=yes --log-file=launcher_%p.log ./launcher node/all_node_config.xml&
