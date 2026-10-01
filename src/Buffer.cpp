#include <fstream>

#include "Buffer.hpp"

namespace sjtu {

Buffer::Buffer(const std::filesystem::path& path){
    //从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
    if(path.empty()) throw std::runtime_error("无法打开文件");
    path_ = path;
    std::ifstream buffer(path);
    if(!buffer) throw std::runtime_error("无法打开文件");
    if(buffer.peek() == std::char_traits<char>::eof()) lines_.push_back("");
    else{
        std::string line;
        while(std::getline(buffer,line)){
            lines_.push_back(line);
        }
    }
    buffer.close();
}

Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path) {
    //新建文件?还是把老的文件底下加几行？
    if(path.empty()) throw std::runtime_error("无法打开文件");
    path_ = path;
    if(lines.empty()) lines_.push_back("");
    else{
        for(const auto& line:lines){
            lines_.push_back(line);
        }
    }
}

std::size_t Buffer::GetLineCount() const {
    //返回文件行数
    return lines_.size();
}

const std::string& Buffer::GetLineAt(std::size_t row) const {
    //返回第row行的内容
    return lines_[row-1];
}


std::string Buffer::GetDisplayName() const {
    //返回文件名,若是新文件,返回"[No Name]"
    if(std::filesystem::exists(path_) && std::filesystem::is_regular_file(path_))  return path_.filename().string();
    return "[No Name]";
    
}

bool Buffer::IsModified() const {
    //返回文件和上次保存比起来是否被修改过
    //未处理文件被删除情况
    std::ifstream buffer(path_);
    std::string line;
    std::vector<std::string> nowlines_;
    if(buffer.peek() == std::char_traits<char>::eof()) nowlines_.push_back("");
    else while(std::getline(buffer,line)) nowlines_.push_back(line);
    if(nowlines_ == lines_) return false;
    return true;
}

void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value) {
    //在第row行第col列插入一个value, 注意越界检查
    if(row > GetLineCount()||row < 1) return;
    if (column > lines_[row-1].size()+1||column < 1) return;
    lines_[row-1].insert(lines_[row-1].begin()+column-1,value);
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
   //在第row行第col列删除一个value
    if(row > GetLineCount()||row < 1) return;
    if (column > lines_[row-1].size()||column < 1) return;
    lines_[row-1].erase(lines_[row-1].begin()+column-1);
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    //注：此处分割人为规定第col列不会留在原处
    if(row > GetLineCount()||row < 1) return;
    if (column > lines_[row-1].size()+1||column < 1) return;
    std::string tail = lines_[row-1].substr(column-1);
    lines_[row-1].erase(column-1);
    lines_.insert(lines_.begin()+row, tail);
}

void Buffer::JoinLine(std::size_t row) {
   //把第row + 1行合并进第row行
    if(row > GetLineCount()-1||row < 1) return;
    std::string tail = lines_[row];
    lines_.erase(lines_.begin()+row);
    lines_[row-1] = lines_[row-1]+tail;
}

void Buffer::Save() {
   //把文件内容保存, 直接调用WriteTo方法
    WriteTo(path_);
}

void Buffer::SaveAs(const std::filesystem::path& path) {
    path_ = path;
    WriteTo(path);
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
   //实际将缓冲区中的内容写入path指向的文件中
    std::ofstream output(path);
    for (size_t i = 0; i < lines_.size(); ++i) {
        if (i > 0) output << '\n';
        output << lines_[i];
    }
}

} // namespace sjtu
