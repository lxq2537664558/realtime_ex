执行脚本前请确保安装了git以及cmake

windows：
请用VS2015 64位命令行工具执行3rd.bat批处理文件

linux：
直接执行3rd.sh shell脚本

另外linux下如果想要检测内存错误信息，请安装 valgrind 开发版 centos下安装命令是 yum install -y valgrind-devel.x86_64
然后启动进程用valgrind_start_node.sh来启动