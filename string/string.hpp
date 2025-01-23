#pragma once
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
class String {
 public:
  String() = default;
  String(unsigned int, char character);
  String(const char* str);
  String(const String& other);
  char& operator[](int index);
  const char& operator[](int index) const;
  char& Front();
  const char& Front() const;
  char& Back();
  const char& Back() const;
  String& operator=(const String& other);
  String& operator+=(const String& other);
  String& operator*=(int number);
  String operator+(const String& other) const;
  String operator*(int number) const;
  ~String();
  const char* Data() const;
  char* Data();
  void Clear();
  void PushBack(char character);
  void PopBack();
  void Resize(unsigned int length);
  void Resize(unsigned int length, char character);
  void Reserve(unsigned int new_cap);
  void ShrinkToFit();
  void Swap(String& other);
  unsigned int Size() const;
  unsigned int Capacity() const;
  bool Empty() const;
  void Realloc(unsigned int new_capacity);
  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;
  bool operator>(const String& other) const;
  bool operator<(const String& other) const;
  bool operator>=(const String& other) const;
  bool operator<=(const String& other) const;
  friend std::istream& operator>>(std::istream&, String&);
  friend std::ostream& operator<<(std::ostream&, const String&);
  String Join(const std::vector<String>& strings) const;
  std::vector<String> Split(const String& delim = " ");

 private:
  static void SplitHealp(std::vector<String>& array, unsigned int& cur_index);
  char* str_ = nullptr;
  unsigned int size_ = 0;
  unsigned int capacity_ = 0;
};
