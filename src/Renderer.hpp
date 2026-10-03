/*
Renderer.hpp
Renderer把Buffer中的内容、Window中的可见区域和Editor的状态拼成一帧字符串.
字符串中既有可见文字,也有控制清屏和光标位置的ANSI转义序列,最终由Terminal输出.
*/
#ifndef MINIVIM_RENDERER_HPP
#define MINIVIM_RENDERER_HPP

#include "Buffer.hpp"
#include "Types.hpp"
#include "Window.hpp"

#include <string>

namespace sjtu {

struct RenderState {
    //绘制底部一行所需的状态,command_不包含冒号,message_保存提示或错误信息
    Mode mode_{Mode::Normal};
    std::string command_;
    std::string message_{"MewMewMew"};
};

class Renderer {
public:
    //生成完整画面,不直接向终端写入,也不修改Buffer和Window
    std::string Render(const Buffer& buffer, const Window& window, const RenderState& state) const;

private:
    //将一行内容展开为可显示的字符,主要处理Tab的宽度
    static std::string ExpandForDisplay(std::string_view line); 
};

} // namespace sjtu

#endif // MINIVIM_RENDERER_HPP
