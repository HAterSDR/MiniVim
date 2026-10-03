#include <fstream>

#include "Buffer.hpp"

namespace sjtu {

Buffer::Buffer(const std::filesystem::path& path){
    //从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
    if(path.empty()) {
        path_ = "";
        lines_.push_back("");
        return;
    }
    path_ = path;
    std::ifstream buffer(path);
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
    if(path.empty()) path = "";
    else path_ = path;
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
    return lines_[row];
}


std::string Buffer::GetDisplayName() const {
    //返回文件名,若是新文件,返回"[No Name]"
    if(std::filesystem::exists(path_) && std::filesystem::is_regular_file(path_))  return path_.filename().string();
    return "[No Name]";
    
}

bool Buffer::IsModified() const {
    //返回文件和上次保存比起来是否被修改过
    //未处理文件被删除情况
    return modified_;
}

void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value) {
    //在第row行第col列插入一个value, 注意越界检查
    if(row >= GetLineCount()) return;
    if (column > lines_[row].size()) return;
    lines_[row].insert(lines_[row].begin()+column,value);
    modified_ = true;
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
   //在第row行第col列删除一个value
    if(row >= GetLineCount()) return;
    if (column >= lines_[row].size()) return;
    lines_[row].erase(lines_[row].begin()+column);
    modified_ = true;
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    //注：此处分割人为规定第col列不会留在原处
    if(row >= GetLineCount()) return;
    if (column > lines_[row].size()) return;
    std::string tail = lines_[row].substr(column);
    lines_[row].erase(column);
    lines_.insert(lines_.begin()+row+1, tail);
    modified_ = true;
}

void Buffer::JoinLine(std::size_t row) {
   //把第row + 1行合并进第row行
    if(row+2 > GetLineCount()) return;
    std::string tail = lines_[row+1];
    lines_.erase(lines_.begin()+row+1);
    lines_[row] = lines_[row]+tail;
    modified_ = true;
}

void Buffer::Save() {
   //把文件内容保存, 直接调用WriteTo方法
    WriteTo(path_);
    modified_ = false;
}

void Buffer::SaveAs(const std::filesystem::path& path) {
    WriteTo(path);
    path_ = path;
    modified_ = false;
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
   //实际将缓冲区中的内容写入path指向的文件中
    std::ofstream output(path, std::ios::out|std::ios::trunc);
    if(!output){
        throw std::runtime_error("无法打开文件");
        return;
    }
    if(lines_.size() == 1 && lines_[0].empty()) return;
    for (size_t i = 0; i < lines_.size(); ++i) {
        output << lines_[i] <<'\n';
    }
    output.close();
}

} // namespace sjtu
