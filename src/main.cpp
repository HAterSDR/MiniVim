/*
main.cpp
程序入口,负责处理命令行参数、启动Editor,以及报告运行中向外抛出的异常.
*/
#include "Editor.hpp"

#include <exception>
#include <filesystem>
#include <iostream>

//你不应该修改该文件
int main(int argc, char* argv[]) {
    //1. 检查参数数量,除程序名外最多接受一个文件路径
    //2. 未提供路径时使用空路径,创建Editor并进入主循环
    //3. 捕获异常并输出错误,失败返回1,正常结束返回0
    //Editor是局部对象,离开try作用域时会析构,其中的Terminal也会恢复终端设置
    if (argc > 2) {
        std::cerr << "MiniVim: too many arguments\n";
        return 1;
    }

    const std::filesystem::path path = argc == 2 ? argv[1] : "";
    try {
        sjtu::Editor editor(path);
        editor.Run();
    } catch (const std::exception& error) {
        std::cerr << "MiniVim: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
