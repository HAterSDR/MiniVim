/*
Terminal.cpp
该文件涉及一些神秘的终端处理,你不需要理解.(虽然我们写了注释)
除了我们明确在Editor中提到的那一处之外,你的所有实现都不应该调用Terminal中的任何接口
你只需要保留我们的实现即可
*/
#include "Terminal.hpp"

#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <system_error>

#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace sjtu {
namespace {

void ThrowSystemError(const char* operation) {
    //把系统调用设置的errno和操作名称包装成异常,让上层统一报告错误
    throw std::system_error(errno, std::generic_category(), operation);
}

} 

Terminal::Terminal() {
    //1. 读取并保存原来的终端设置
    //2. 在副本上关闭echo、按行输入和部分自动字符处理,使程序可以逐字节处理按键
    //3. 设置VMIN=0、VTIME=1,让一次read在没有输入时等待约0.1秒后返回

    if (::tcgetattr(STDIN_FILENO, &original_) == -1) {
        ThrowSystemError("tcgetattr");
    }

    termios raw = original_;
    tcflag_t inputFlags = BRKINT | ICRNL | INPCK | ISTRIP | IXON;
    tcflag_t outputFlags = OPOST;
    tcflag_t localFlags = ECHO | ICANON | IEXTEN | ISIG;

    raw.c_iflag &= ~inputFlags;
    raw.c_oflag &= ~outputFlags;
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~localFlags;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        ThrowSystemError("tcsetattr");
    }
}

Terminal::~Terminal() noexcept {
    ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
}

KeyEvent Terminal::ReadKey() {
    //1. 读取首字节,把回车和退格转成对应KeyCode,普通字节包装成Character
    //2. 遇到ESC时尝试读取后续字节,没有完整后续输入时按Escape处理
    //3. 按ESC [或ESC O开头的序列识别方向键等特殊键
    //4. 带数字的序列还要检查末尾的~,无法识别的序列统一返回Escape
    //例如方向上键通常是ESC[A,Delete通常是ESC[3~;它们都不是单个字符
    auto first = ReadByte();
    switch (first) {
    case '\r':
    case '\n':
        return {KeyCode::Enter, 0};
    case 0x7FU:
    case '\b':
        return {KeyCode::Backspace, 0};
    case 0x1BU:
        break;
    default:
        return KeyEvent::Character(first);
    }

    auto second = TryReadByte();
    if (!second.has_value()) {
        return {KeyCode::Escape, 0};
    }
    auto third = TryReadByte();
    if (!third.has_value()) {
        return {KeyCode::Escape, 0};
    }

    if (*second == '[') {
        if (*third >= '0' && *third <= '9') {
            auto fourth = TryReadByte();
            if (!fourth.has_value() || *fourth != '~') {
                return {KeyCode::Escape, 0};
            }
            switch (*third) {
            case '1':
            case '7':
                return {KeyCode::Home, 0};
            case '3':
                return {KeyCode::Delete, 0};
            case '4':
            case '8':
                return {KeyCode::End, 0};
            case '5':
                return {KeyCode::PageUp, 0};
            case '6':
                return {KeyCode::PageDown, 0};
            default:
                return {KeyCode::Escape, 0};
            }
        }

        switch (*third) {
        case 'A':
            return {KeyCode::ArrowUp, 0};
        case 'B':
            return {KeyCode::ArrowDown, 0};
        case 'C':
            return {KeyCode::ArrowRight, 0};
        case 'D':
            return {KeyCode::ArrowLeft, 0};
        case 'H':
            return {KeyCode::Home, 0};
        case 'F':
            return {KeyCode::End, 0};
        default:
            return {KeyCode::Escape, 0};
        }
    }

    if (*second == 'O') {
        if (*third == 'H') {
            return {KeyCode::Home, 0};
        }
        if (*third == 'F') {
            return {KeyCode::End, 0};
        }
    }
    return {KeyCode::Escape, 0};
}

ScreenSize Terminal::GetScreenSize() {
    //优先用ioctl取得终端行列数;失败时将光标向右下移动到边界,再查询光标位置
    //备用路径会移动终端光标,下一次绘制会重新定位它
    winsize size{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_row > 0 && size.ws_col > 0) {
        return {static_cast<size_t>(size.ws_row), static_cast<size_t>(size.ws_col)};
    }

    WriteOutput("\x1b[999C\x1b[999B");
    return QueryCursorPosition();
}

void Terminal::WriteOutput(std::string_view output) {
    //write可能只写出一部分内容,需要累计已写字节数,循环写完剩余部分
    //被信号中断(EINTR)时重试,其他失败交给ThrowSystemError
    size_t written = 0;
    while (written < output.size()) {
        const auto result = ::write(STDOUT_FILENO, output.data() + written, output.size() - written);
        if (result > 0) {
            written += static_cast<std::size_t>(result);
            continue;
        }
        if (result == -1 && errno == EINTR) {continue;}
        ThrowSystemError("write");
    }
}

void Terminal::ClearScreen() {
    //输出ANSI转义序列:ESC[2J清屏,ESC[H把光标放回左上角
    WriteOutput("\x1b[2J\x1b[H");
}

char Terminal::ReadByte() {
    //循环读取一个字节,超时或暂时不可读时继续等,被信号中断时重试
    //其他读取错误抛出异常;这个接口用于等待用户按下一个新键
    while (true) {
        char value = 0;
        const auto result = ::read(STDIN_FILENO, &value, 1);
        if (result == 1) {return value;}
        if (result == -1 && errno != EAGAIN && errno != EINTR) {ThrowSystemError("read");}
    }
}

std::optional<char> Terminal::TryReadByte() {
    //等待后续字节最多10毫秒,超时或暂时不可读时返回nullopt,被信号中断时重试
    //它用来等待转义序列的后续字节,避免把单独按下Escape误当成永远没读完的特殊键
    while (true) {
        pollfd fd{STDIN_FILENO, POLLIN, 0};
        const auto ready = ::poll(&fd, 1, 10);
        if (ready == 0) { return std::nullopt; }
        if (ready == -1) {
            if (errno == EINTR) { continue; }
            ThrowSystemError("poll");
        }

        char value;
        const auto result = ::read(STDIN_FILENO, &value, 1);
        if (result == 1) {return value;}
        if (result == 0 || (result == -1 && errno == EAGAIN)) {return std::nullopt;}
        if (result == -1 && errno == EINTR) { continue; }
        ThrowSystemError("read");
    }
}

ScreenSize Terminal::QueryCursorPosition() {
    //1. 发送ESC[6n请求光标位置,终端通常回复ESC[行号;列号R
    //2. 逐字节读取,遇到R、超时或数组即将写满时停止,记得预留字符串结束符的位置
    //3. 检查前缀并解析两个正整数,无法解析时抛出异常
    //返回的是从1开始的光标坐标;调用方先将光标移到右下角,才把它当作屏幕行列数
    WriteOutput("\x1b[6n");

    char response[32];
    size_t length = 0;
    while (length + 1 < sizeof(response)) {
        const auto byte = TryReadByte();
        if (!byte.has_value()) {break;}
        response[length] = *byte;
        if (response[length] == 'R') {
            ++length;
            break;
        }
        ++length;
    }
    response[length] = '\0';

    int rows = 0;
    int columns = 0;
    if (length < 4 || response[0] != '\x1b' || response[1] != '[' || std::sscanf(response + 2, "%d;%d", &rows, &columns) != 2
     || rows <= 0 || columns <= 0) {
        throw std::runtime_error("cannot determine terminal size");
    }
    return {static_cast<size_t>(rows), static_cast<size_t>(columns)};
}

} // namespace sjtu
