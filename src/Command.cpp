#include "Command.hpp"

namespace sjtu {

EditorAction NormalModeParser::Feed(KeyEvent key) {
    //根据传入的key生成Action,在Basic部分中你应该直接调用GenerateMotion
    //注意清空当前状态
    //注意一万个边界
    //注意把pending放在开头判断
    if (key.code_ == KeyCode::Escape) {
        ResetState();
        return {};
    }
    
    if (key.code_ == KeyCode::Character) {
        auto value = key.value_;
        if(pending_){
            if(pending_.value()=='g'){
            switch(value){
                case 'g':{
                    auto action = GenerateCommand(ActionKind::JumpToLineStart,
                                                   pending_count == 0 ? 1 : pending_count);
                    ResetState();
                    return action;
                }
                default:break;
            }
        }
        }
        if(value>='0' && value<='9'){
            size_t digit = value - '0';
            if(digit == 0 && pending_count == 0){
                auto action = GenerateCommand(ActionKind::JumpToColStart);
                ResetState();
                return action;
            }
            else{
                pending_count = pending_count*10 + digit;
                count = pending_count;
                if(digit == 0) return {};
            }
        };
        switch (value) {
            case 'h':{
                auto action = GenerateMotion(Motion::Left,count);
                ResetState();
                return action;
            }
            case 'j':{
                auto action = GenerateMotion(Motion::Down,count);
                ResetState();
                return action;
            }
            case 'k':{
                auto action = GenerateMotion(Motion::Up,count);
                ResetState();
                return action;
            }
            case 'l':{
                auto action = GenerateMotion(Motion::Right,count);
                ResetState();
                return action;
            }
            case 'i':{
                auto action = GenerateCommand(ActionKind::InsertBefore,count);
                ResetState();
                return action;
            }
            case 'I':{
                auto action = GenerateCommand(ActionKind::InsertStart,count);
                ResetState();
                return action;
            }
            case 'a':{
                auto action = GenerateCommand(ActionKind::InsertAfter,count);
                ResetState();
                return action;
            }
            case 'A':{
                auto action = GenerateCommand(ActionKind::InsertLast,count);
                ResetState();
                return action;
            }
            case 'o':{
                auto action = GenerateCommand(ActionKind::OpenNext,count);
                ResetState();
                return action;
            }
            case 'O':{
                auto action = GenerateCommand(ActionKind::OpenPrev,count);
                ResetState();
                return action;
            }
            case ':':{
                auto action = GenerateCommand(ActionKind::EnterCommandLine);
                ResetState();
                return action;
            }
            case 'x':{
                auto action = GenerateCommand(ActionKind::Delete,count);
                ResetState();
                return action;
            }
            case 'X':{
                auto action = GenerateCommand(ActionKind::DeleteBefore,count);
                ResetState();
                return action;
            }
            case 'G':{
                auto action = GenerateCommand(ActionKind::JumpToLineEnd,pending_count);
                ResetState();
                return action;
            }
            case 'g':pending_='g';break;
            case '0':{
                auto action = GenerateCommand(ActionKind::JumpToColStart,pending_count);
                ResetState();
                return action;
            }
            case '$':{
                auto action = GenerateCommand(ActionKind::JumpToColEnd,pending_count);
                ResetState();
                return action;
            }
            case '^':{
                auto action = GenerateCommand(ActionKind::JumpToRealStart);
                ResetState();
                return action;
            };
            case 'r':{

            }
            //TODO: 今天得完成i, I, a, A, o, O,r;
            //力竭了只做了前六个
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


EditorAction NormalModeParser::GenerateMotion(Motion motion, size_t count ) {
    return {ActionKind::Move, motion, std::optional<Count>(Count {count})};
}

EditorAction NormalModeParser::GenerateCommand(ActionKind kind, size_t count ) {
    return {kind, std::nullopt,std::optional<Count>(Count {count})};
}
EditorAction NormalModeParser::GenerateSpecialMotion(ActionKind kind){
    return {};
}
void NormalModeParser::ResetState() {
    count = 1;
    pending_count = 0;
    pending_ = std::nullopt;
    // 清空其他所有待定前缀
}
} // namespace sjtu
