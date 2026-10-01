#include "Command.hpp"

namespace sjtu {

EditorAction NormalModeParser::Feed(KeyEvent key) {
    //根据传入的key生成Action,在Basic部分中你应该直接调用GenerateMotion
    if (key.code_ == KeyCode::Escape) {
        return {};
    }


    if (key.code_ == KeyCode::Character) {
        auto value = key.value_;
        switch (value) {
            case 'h':return GenerateMotion(Motion::Left);break;
            case 'j':return GenerateMotion(Motion::Down);break;
            case 'k':return GenerateMotion(Motion::Up);break;
            case 'l':return GenerateMotion(Motion::Right);break;
            case 'i':return GenerateCommand(ActionKind::InsertBefore);break;
            case 'a':return GenerateCommand(ActionKind::InsertAfter);break;
            case ':':return GenerateCommand(ActionKind::EnterCommandLine);
        default:
            break;
        }
        return {};
    }

    //switch (key.code_) {
    //case KeyCode::ArrowLeft:
        //return GenerateMotion(Motion::Left);
    //case KeyCode::ArrowRight:
        //return GenerateMotion(Motion::Right);
    //case KeyCode::ArrowUp:
        //return GenerateMotion(Motion::Up);
    //case KeyCode::ArrowDown:
        //return GenerateMotion(Motion::Down);
    //default:
        //return {};
    //}
    //虽然不要求这些按键,但是我们给你的Terminal.hpp可以处理这些你键盘上的特殊按键并把他们放在了keycode里
    //在Vim中,它们对应着hjkl.

    return {};
}


EditorAction NormalModeParser::GenerateMotion(Motion motion) {
    return {ActionKind::Move, motion};
}

EditorAction NormalModeParser::GenerateCommand(ActionKind kind) {
    return {kind, std::nullopt};
}

} // namespace sjtu
