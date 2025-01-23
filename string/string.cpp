#include "string.hpp"
String::String(unsigned int size, char character)
    : size_(size), capacity_(size_ * 2) {
  str_ = new char[capacity_];
  std::fill(str_, str_ + size_, character);
  str_[size_] = '\0';
}
String::String(const char* str) : size_(strlen(str)), capacity_(size_ * 2 + 1) {
  if (capacity_ > 0) {
    str_ = new char[capacity_];
    std::copy(str, str + size_, str_);
    str_[size_] = '\0';
  } else {
    str_ = nullptr;
  }
}
String::String(const String& other) : String(other.Data()){};
char& String::operator[](int index) { return str_[index]; }
const char& String::operator[](int index) const { return str_[index]; }
char& String::Front() { return str_[0]; }
const char& String::Front() const { return str_[0]; }
char& String::Back() { return str_[size_ - 1]; }
const char& String::Back() const { return str_[size_ - 1]; }
String& String::operator=(const String& other) {
  if (&other != this) {
    delete[] str_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    str_ = new char[capacity_];
    std::copy(other.str_, other.str_ + other.size_, str_);
    str_[size_] = '\0';
  }
  return *this;
}
void String::Realloc(unsigned int new_capacity) {
  String newstr;
  newstr.str_ = new char[new_capacity];
  std::copy(str_, str_ + size_, newstr.str_);
  std::swap(str_, newstr.str_);
}
String& String::operator+=(const String& other) {
  Realloc(2 * (size_ + other.size_) + 1);
  for (unsigned int index = 0; index < other.size_; ++index) {
    str_[index + size_] = other.str_[index];
  }
  size_ += other.size_;
  capacity_ = size_ * 2;
  str_[size_] = '\0';
  return *this;
}
String& String::operator*=(int number) {
  if (number >= 0) {
    Realloc(2 * size_ * number + 1);
    for (unsigned int index = size_; index < size_ * number; ++index) {
      str_[index] = str_[index - size_];
    }
    size_ *= number;
    capacity_ = size_ * 2;
    str_[size_] = '\0';
  }
  return *this;
}
String String::operator+(const String& other) const {
  String newstr;
  newstr += *this;
  newstr += other;
  return newstr;
}
String String::operator*(int number) const {
  String newstr;
  if (number > 0) {
    newstr.capacity_ = 2 * size_ * number;
    newstr.str_ = new char[newstr.capacity_ + 1];
    std::copy(str_, str_ + size_, newstr.str_);
    for (unsigned int index = size_; index < size_ * number; index++) {
      newstr.str_[index] = newstr.str_[index - size_];
    }
    newstr.size_ = size_ * number;
    newstr[newstr.size_] = '\0';
  }
  return newstr;
}
String::~String() { delete[] str_; }
const char* String::Data() const { return str_; }
char* String::Data() { return str_; }
void String::Clear() { size_ = 0; }
void String::PushBack(char character) {
  if (size_ + 1 >= capacity_) {
    if (size_ == 0) {
      str_ = new char[2];
      capacity_ = 1;
    } else {
      Realloc(2 * size_ + 1);
      capacity_ = 2 * size_;
    }
  }
  str_[size_] = character;
  ++size_;
  str_[size_] = '\0';
}
void String::PopBack() {
  if (size_ != 0) {
    --size_;
    str_[size_] = '\0';
  }
}
void String::Resize(unsigned int length) {
  if (size_ < length) {
    Realloc(length + 1);
    capacity_ = length;
  }
  size_ = length;
  str_[size_] = '\0';
}
void String::Resize(unsigned int length, char character) {
  if (size_ < length) {
    Realloc(length + 1);
    for (unsigned int index = size_; index < length; ++index) {
      str_[index] = character;
    }
    capacity_ = length;
  }
  size_ = length;
  str_[size_] = '\0';
}
void String::Reserve(unsigned int new_cap) {
  if (capacity_ < new_cap) {
    capacity_ = new_cap;
    Realloc(capacity_);
  }
}
void String::ShrinkToFit() {
  capacity_ = size_;
  Realloc(capacity_);
}
void String::Swap(String& other) {
  std::swap(str_, other.str_);
  std::swap(capacity_, other.capacity_);
  std::swap(size_, other.size_);
}
unsigned int String::Size() const { return size_; }
unsigned int String::Capacity() const { return capacity_; }
bool String::Empty() const { return size_ == 0; }
bool String::operator==(const String& other) const {
  if (size_ != other.size_) {
    return false;
  }
  for (unsigned int index = 0; index < size_; ++index) {
    if (str_[index] != other.str_[index]) {
      return false;
    }
  }
  return true;
}
bool String::operator!=(const String& other) const {
  return !(operator==(other));
}
bool String::operator>(const String& other) const {
  for (unsigned int index = 0; index < std::min(size_, other.size_); ++index) {
    if (str_[index] > other.str_[index]) {
      return true;
    }
    if (str_[index] < other.str_[index]) {
      return false;
    }
  }
  return size_ > other.size_;
}
bool String::operator<(const String& other) const {
  return other.operator>(*this);
}
bool String::operator>=(const String& other) const {
  return !(operator<(other));
}
bool String::operator<=(const String& other) const {
  return !(operator>(other));
}
String String::Join(const std::vector<String>& strings) const {
  String newstr;
  if (!strings.empty()) {
    for (unsigned index = 0; index < strings.size() - 1; index++) {
      newstr += strings[index];
      newstr += *this;
    }
    newstr += strings[strings.size() - 1];
  }
  return newstr;
}
void String::SplitHealp(std::vector<String>& array, unsigned int& cur_index) {
  array[cur_index].PopBack();
  array.emplace_back();
  cur_index += 1;
}
std::vector<String> String::Split(const String& delim) {
  unsigned int delim_index = 0;
  unsigned int cur_index = 0;
  std::vector<String> ans;
  ans.emplace_back();
  for (unsigned int index = 0; index < size_; index++) {
    ans[cur_index].PushBack(str_[index]);
    if (delim_index == 0) {
      if (str_[index] == delim[0]) {
        if (delim_index == delim.size_ - 1) {
          SplitHealp(ans, cur_index);
        } else {
          delim_index += 1;
        }
      }
    } else if (str_[index] == delim[delim_index]) {
      delim_index += 1;
      if (delim_index - 1 == delim.size_ - 1) {
        for (unsigned int i = 0; i < delim.size_ - 1; i++) {
          ans[cur_index].PopBack();
        }
        SplitHealp(ans, cur_index);
        delim_index = 0;
      }
    } else {
      delim_index = 0;
    }
  }
  return ans;
}
std::ostream& operator<<(std::ostream& out, const String& output) {
  for (unsigned int index = 0; index < output.size_; index++) {
    out << output.str_[index];
  }
  return out;
}
std::istream& operator>>(std::istream& in, String& to_in) {
  char input_character;
  while (in.get(input_character)) {
    if (input_character == '\n' || input_character == ' ' && to_in.Size() > 0) {
      return in;
    }
    if (input_character != ' ') {
      to_in.PushBack(input_character);
    }
  }
  return in;
}