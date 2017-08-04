这里protobuf版本使用3.x版本
执行脚本前请确保安装了git

windows：
请用VS2015 64位命令行工具执行protobuf.bat批处理文件

另外这里选择将pb编译成动态库的形式，处于以下考虑：
1.静态库protobuf自动编译crt只支持mt选项，这个跟我们大部分项目的crt选项不符合，而且protobuf版本使用3是特意做了这个事情，意图不明，不好贸然改成md选项。
2.我们的框架是以dll的形式存在，并且逻辑代码也被包装成dll，为了保持protobuf版本使用3的独立性，这里编译成dll，跨dll内存问题在md选项下是没有问题的。

linux：
直接执行protobuf.sh shell脚本

linux下也是编译成so形式，并且在linux不存在跨so内存问题