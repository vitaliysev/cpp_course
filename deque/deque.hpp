#include <algorithm>
#include <exception>
#include <iostream>
#include <random>
#include <vector>
const uint8_t kBucketSize = 5;
template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  Deque() = default;
  Deque(const Allocator& alloc);
  Deque(const Deque& other);
  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other);
  Deque(Deque&& other);
  Deque(size_t count, const Allocator& alloc = Allocator());
  Deque(size_t count, const T& value, const Allocator& alloc = Allocator());
  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  ~Deque();
  Allocator get_allocator() const { return alloc_; }
  size_t size() const { return size_; }
  bool empty();
  T& operator[](size_t index);
  const T& operator[](size_t index) const;
  T& at(size_t index);
  const T& at(size_t index) const;
  void push_back(const T& item);
  void push_back(T&& item);
  template <class... Args>
  void emplace_back(Args&&... args);
  template <class... Args>
  void emplace_front(Args&&... args);
  void pop_back();
  void push_front(const T& elem);
  void push_front(T&& elem);
  void pop_front();
  template <bool IsConst>
  class Iterator;

  using iterator = Deque::Iterator<false>;
  using const_iterator = Deque::Iterator<true>;
  using reverse_iterator = std::reverse_iterator<Deque::Iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<Deque::Iterator<true>>;
  iterator begin();
  iterator end();
  const_iterator cend();
  const_iterator cbegin();
  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
  reverse_iterator crbegin() { return std::make_reverse_iterator(cend()); }
  reverse_iterator crend() { return std::make_reverse_iterator(cbegin()); }
  void insert(iterator iter, T&& item);
  void insert(iterator it1, const T& item);
  void erase(iterator it1);

 private:
  Allocator alloc_;
  using alloc_traits = std::allocator_traits<Allocator>;
  using bucket_alloc = typename alloc_traits::template rebind_alloc<T*>;
  using bucket_alloc_traits = typename alloc_traits::template rebind_traits<T*>;
  bucket_alloc outer_alloc_;
  size_t size_ = 0;
  uint8_t inner_start_ = 0;
  uint8_t inner_end_ = 0;
  size_t outer_size_ = 0;
  size_t outer_start_ = 0;
  size_t outer_end_ = 0;
  size_t outer_capacity_ = 0;
  size_t memory_outer_end_ = 0;
  size_t memory_outer_start_ = 0;
  T** outer_ = nullptr;
  void reserve();
  void destroy(Deque& tmp, size_t out_ind, size_t in_ind, size_t fin_ind);
  void constructor_destroy(size_t out_ind, size_t in_ind, size_t index);
  void swap(Deque& first, Deque& second);
  void initialize(Deque& deque, const Deque& other);
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  Iterator(T** outer_pointer, size_t out_ind, size_t in_ind);
  reference operator*() const { return outer_[outer_index_][inner_index_]; }
  pointer operator->() const { return outer_[outer_index_] + inner_index_; }
  Iterator& operator+=(int n);
  Iterator& operator-=(int n);
  Iterator operator-(int n) const;
  Iterator operator+(int n) const;
  Iterator operator++(int);
  Iterator& operator++();
  Iterator operator--(int);
  Iterator operator--();
  bool operator==(const Iterator& other) const {
    return (outer_index_ == other.outer_index_ &&
            inner_index_ == other.inner_index_);
  }
  bool operator!=(const Iterator& other) { return !(*this == other); }
  bool operator<(const Iterator& other) {
    return outer_index_ * kBucketSize + inner_index_ <
           other.outer_index_ * kBucketSize + other.inner_index_;
  }
  bool operator>(const Iterator& other) {
    return outer_index_ * kBucketSize + inner_index_ >
           other.outer_index_ * kBucketSize + other.inner_index_;
  }
  bool operator<=(const Iterator& other) {
    return outer_index_ * kBucketSize + inner_index_ <=
           other.outer_index_ * kBucketSize + other.inner_index_;
  }
  bool operator>=(const Iterator& other) {
    return outer_index_ * kBucketSize + inner_index_ >=
           other.outer_index_ * kBucketSize + other.inner_index_;
  }
  difference_type operator-(Iterator other) {
    return outer_index_ * kBucketSize + inner_index_ -
           other.outer_index_ * kBucketSize - other.inner_index_;
  }

 private:
  difference_type inner_index_;
  difference_type outer_index_;
  T** outer_;
};
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Allocator& alloc)
    : alloc_(alloc), outer_alloc_(alloc) {}
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : alloc_(alloc),
      outer_alloc_(alloc),
      size_(init.size()),
      outer_size_((size_ + kBucketSize - 1) / kBucketSize),
      outer_capacity_(outer_size_ * 2),
      outer_(bucket_alloc_traits::allocate(outer_alloc_, outer_capacity_)),
      outer_start_(outer_size_ / 2),
      outer_end_(outer_start_ + outer_size_ - 1),
      inner_end_((size_ - 1) % kBucketSize),
      memory_outer_end_(outer_end_),
      memory_outer_start_(outer_start_) {
  for (size_t out_ind = 0; out_ind < outer_size_; ++out_ind) {
    outer_[outer_start_ + out_ind] =
        alloc_traits::allocate(alloc_, kBucketSize);
  }
  size_t out_ind = 0;
  size_t in_ind = 0;
  size_t index = 0;
  auto iter = init.begin();
  for (; out_ind < outer_size_ - 1; ++out_ind) {
    in_ind = 0;
    for (; in_ind < kBucketSize; ++in_ind) {
      alloc_traits::construct(alloc_, outer_[out_ind + outer_start_] + in_ind,
                              *iter);
      ++iter;
    }
  }
  for (; index <= inner_end_; ++index) {
    alloc_traits::construct(alloc_, outer_[outer_end_] + index, *iter);
    ++iter;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    Deque<T, Allocator>&& other) {
  if (this == &other) {
    return *this;
  }
  if (bucket_alloc_traits::propagate_on_container_move_assignment::value) {
    outer_alloc_ = other.outer_alloc_;
  }
  if (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = other.alloc_;
  }
  outer_size_ = other.outer_size_;
  outer_capacity_ = other.outer_capacity_;
  outer_start_ = other.outer_start_;
  outer_end_ = other.outer_end_;
  inner_end_ = other.inner_end_;
  inner_start_ = other.inner_start_;
  size_ = other.size_;
  memory_outer_end_ = other.memory_outer_end_;
  memory_outer_start_ = other.memory_outer_start_;
  if (other.alloc_ == alloc_) {
    outer_ = other.outer_;
    other.outer_ = nullptr;
    other.outer_size_ = 0;
    other.outer_capacity_ = 0;
    other.outer_start_ = 0;
    other.outer_end_ = 0;
    other.inner_end_ = 0;
    other.inner_start_ = 0;
    other.size_ = 0;
    other.memory_outer_end_ = 0;
    other.memory_outer_start_ = 0;
  } else {
    size_t border = kBucketSize - 1;
    size_t fin_ind = 0;
    outer_ = bucket_alloc_traits::allocate(outer_alloc_, outer_capacity_);
    for (size_t i = 0; i < outer_size_; ++i) {
      outer_[outer_start_ + i] = alloc_traits::allocate(alloc_, kBucketSize);
    }
    border = (outer_end_ == outer_start_ ? inner_end_ : border);
    for (size_t st_ind = inner_start_; st_ind <= border; ++st_ind) {
      alloc_traits::construct(alloc_, outer_[outer_start_] + st_ind,
                              std::move(other.outer_[outer_start_][st_ind]));
    }
    if (outer_start_ != outer_end_) {
      for (size_t out_ind = outer_start_ + 1; out_ind < outer_end_; ++out_ind) {
        size_t in_ind = 0;
        for (; in_ind < kBucketSize; ++in_ind) {
          alloc_traits::construct(alloc_, outer_[out_ind] + in_ind,
                                  std::move(other.outer_[out_ind][in_ind]));
        }
      }
      for (; fin_ind <= inner_end_; ++fin_ind) {
        alloc_traits::construct(alloc_, outer_[outer_end_] + fin_ind,
                                std::move(other.outer_[outer_end_][fin_ind]));
      }
    }
  }
  return *this;
}
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque<T, Allocator>&& other)
    : outer_alloc_(std::move(other.outer_alloc_)),
      alloc_(std::move(other.alloc_)),
      outer_(other.outer_),
      outer_size_(other.outer_size_),
      outer_capacity_(other.outer_capacity_),
      outer_start_(other.outer_start_),
      outer_end_(other.outer_end_),
      inner_end_(other.inner_end_),
      inner_start_(other.inner_start_),
      size_(other.size_),
      memory_outer_end_(other.memory_outer_end_),
      memory_outer_start_(other.memory_outer_start_) {
  other.outer_ = nullptr;
  other.outer_size_ = 0;
  other.outer_capacity_ = 0;
  other.outer_start_ = 0;
  other.outer_end_ = 0;
  other.inner_end_ = 0;
  other.inner_start_ = 0;
  other.size_ = 0;
  other.memory_outer_end_ = 0;
  other.memory_outer_start_ = 0;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque<T, Allocator>& other)
    : outer_alloc_(bucket_alloc_traits::select_on_container_copy_construction(
          other.outer_alloc_)),
      alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)),
      outer_size_(other.outer_size_),
      outer_capacity_(other.outer_capacity_),
      outer_start_(other.outer_start_),
      outer_end_(other.outer_end_),
      inner_end_(other.inner_end_),
      inner_start_(other.inner_start_),
      size_(other.size_),
      memory_outer_end_(other.memory_outer_end_),
      memory_outer_start_(other.memory_outer_start_) {
  if (other.outer_ == nullptr) {
    return;
  }
  size_t st_ind = inner_start_;
  size_t out_ind = outer_start_ + 1;
  size_t in_ind = 0;
  size_t border = kBucketSize - 1;
  size_t fin_ind = 0;
  try {
    outer_ = bucket_alloc_traits::allocate(outer_alloc_, other.outer_capacity_);
    for (size_t i = 0; i < outer_size_; ++i) {
      outer_[outer_start_ + i] = alloc_traits::allocate(alloc_, kBucketSize);
    }
    border = (outer_end_ == outer_start_ ? inner_end_ : border);
    for (; st_ind <= border; ++st_ind) {
      alloc_traits::construct(alloc_, outer_[outer_start_] + st_ind,
                              other.outer_[other.outer_start_][st_ind]);
    }
    if (outer_start_ != outer_end_) {
      for (; out_ind < outer_end_; ++out_ind) {
        in_ind = 0;
        for (; in_ind < kBucketSize; ++in_ind) {
          alloc_traits::construct(alloc_, outer_[out_ind] + in_ind,
                                  other.outer_[out_ind][in_ind]);
        }
      }
      for (; fin_ind <= inner_end_; ++fin_ind) {
        alloc_traits::construct(alloc_, outer_[outer_end_] + fin_ind,
                                other.outer_[other.outer_end_][fin_ind]);
      }
    }
  } catch (...) {
    for (size_t i = inner_start_; i < st_ind; ++i) {
      alloc_traits::destroy(alloc_, outer_[outer_start_] + i);
    }
    destroy(*this, out_ind, in_ind, fin_ind);
    bucket_alloc_traits::deallocate(outer_alloc_, outer_, outer_capacity_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(
    const Deque<T, Allocator>& other) {
  if (&other == this) {
    return *this;
  }
  if (bucket_alloc_traits::propagate_on_container_copy_assignment::value) {
    outer_alloc_ = other.outer_alloc_;
    alloc_ = other.alloc_;
  }
  Deque<T, Allocator> tmp(other);
  swap(*this, tmp);
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc),
      outer_alloc_(alloc),
      size_(count),
      outer_size_((count + kBucketSize - 1) / kBucketSize),
      outer_capacity_(outer_size_ * 2),
      outer_(bucket_alloc_traits::allocate(outer_alloc_, outer_capacity_)),
      outer_start_(outer_size_ / 2),
      outer_end_(outer_start_ + outer_size_ - 1),
      inner_end_((count - 1) % kBucketSize),
      memory_outer_end_(outer_end_),
      memory_outer_start_(outer_start_) {
  for (size_t out_ind = 0; out_ind < outer_size_; ++out_ind) {
    outer_[outer_start_ + out_ind] =
        alloc_traits::allocate(alloc_, kBucketSize);
  }
  size_t out_ind = 0;
  size_t in_ind = 0;
  size_t index = 0;
  try {
    for (; out_ind < outer_size_ - 1; ++out_ind) {
      in_ind = 0;
      for (; in_ind < kBucketSize; ++in_ind) {
        alloc_traits::construct(alloc_, outer_[out_ind + outer_start_] + in_ind,
                                value);
      }
    }
    for (; index <= inner_end_; ++index) {
      alloc_traits::construct(alloc_, outer_[outer_end_] + index, value);
    }
  } catch (...) {
    constructor_destroy(out_ind, in_ind, index);
    throw;
  }
}
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : alloc_(alloc),
      outer_alloc_(alloc),
      size_(count),
      outer_size_((count + kBucketSize - 1) / kBucketSize),
      outer_capacity_(outer_size_ * 2),
      outer_(bucket_alloc_traits::allocate(outer_alloc_, outer_capacity_)),
      outer_start_(outer_size_ / 2),
      outer_end_(outer_start_ + outer_size_ - 1),
      inner_end_((count - 1) % kBucketSize),
      memory_outer_end_(outer_end_),
      memory_outer_start_(outer_start_) {
  for (size_t out_ind = 0; out_ind < outer_size_; ++out_ind) {
    outer_[outer_start_ + out_ind] =
        alloc_traits::allocate(alloc_, kBucketSize);
  }
  size_t out_ind = 0;
  size_t in_ind = 0;
  size_t index = 0;
  try {
    for (; out_ind < outer_size_ - 1; ++out_ind) {
      in_ind = 0;
      for (; in_ind < kBucketSize; ++in_ind) {
        alloc_traits::construct(alloc_,
                                outer_[out_ind + outer_start_] + in_ind);
      }
    }
    for (; index <= inner_end_; ++index) {
      alloc_traits::construct(alloc_, outer_[outer_end_] + index);
    }
  } catch (...) {
    constructor_destroy(out_ind, in_ind, index);
    throw;
  }
}
template <typename T, typename Allocator>
Deque<T, Allocator>::~Deque() {
  if (outer_ != nullptr) {
    if (memory_outer_end_ == memory_outer_start_) {
      for (size_t index = inner_start_; index <= inner_end_; ++index) {
        alloc_traits::destroy(alloc_, outer_[memory_outer_end_] + index);
      }
      alloc_traits::deallocate(alloc_, outer_[memory_outer_end_], kBucketSize);
    } else {
      for (size_t index = inner_start_; index < kBucketSize; ++index) {
        alloc_traits::destroy(alloc_, outer_[memory_outer_start_] + index);
      }
      alloc_traits::deallocate(alloc_, outer_[memory_outer_start_],
                               kBucketSize);
      if (memory_outer_end_ != memory_outer_start_) {
        for (size_t index = 0; index <= inner_end_; ++index) {
          alloc_traits::destroy(alloc_, outer_[memory_outer_end_] + index);
        }
        alloc_traits::deallocate(alloc_, outer_[memory_outer_end_],
                                 kBucketSize);
      }
      for (size_t out_ind = memory_outer_start_ + 1;
           out_ind < memory_outer_end_; ++out_ind) {
        for (size_t in_ind = 0; in_ind < kBucketSize; ++in_ind) {
          alloc_traits::destroy(alloc_, outer_[out_ind] + in_ind);
        }
        alloc_traits::deallocate(alloc_, outer_[out_ind], kBucketSize);
      }
    }
    bucket_alloc_traits::deallocate(outer_alloc_, outer_, outer_capacity_);
  }
}
template <typename T, typename Allocator>
bool Deque<T, Allocator>::empty() {
  return size_ == 0;
}
template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](size_t index) {
  return outer_[outer_start_ + (index + inner_start_) / kBucketSize]
               [(index + inner_start_) % kBucketSize];
}
template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](size_t index) const {
  return outer_[outer_start_ + (index + inner_start_) / kBucketSize]
               [(index + inner_start_) % kBucketSize];
}
template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t index) {
  if (index >= size_) {
    throw std::out_of_range("");
  }
  return outer_[outer_start_ + (index + inner_start_) / kBucketSize]
               [(index + inner_start_) % kBucketSize];
}
template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t index) const {
  if (index >= size_) {
    throw std::out_of_range("");
  }
  return outer_[outer_start_ + (index + inner_start_) / kBucketSize]
               [(index + inner_start_) % kBucketSize];
}

template <typename T, typename Allocator>
template <class... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (outer_ == nullptr) {
    outer_ = bucket_alloc_traits::allocate(outer_alloc_, 2);
    outer_[0] = alloc_traits::allocate(alloc_, kBucketSize);
    outer_capacity_ = 2;
    outer_size_ = 1;
    size_++;
    try {
      alloc_traits::construct(alloc_, outer_[outer_end_] + inner_end_,
                              std::forward<Args>(args)...);
    } catch (...) {
      this->~Deque();
      *this = Deque();
      throw;
    }
  } else {
    if (outer_end_ == outer_capacity_ - 1 && inner_end_ == 4) {
      reserve();
    }
    if (inner_end_ == kBucketSize - 1) {
      if (outer_end_ == memory_outer_end_) {
        outer_[outer_end_ + 1] = alloc_traits::allocate(alloc_, kBucketSize);
        memory_outer_end_++;
      }
      outer_end_++;
      outer_size_++;
      inner_end_ = 0;
    } else {
      inner_end_++;
    }
    size_++;
    try {
      alloc_traits::construct(alloc_, outer_[outer_end_] + inner_end_,
                              std::forward<Args>(args)...);
    } catch (...) {
      outer_end_ = (inner_end_ == 0 ? outer_end_ - 1 : outer_end_);
      inner_end_ = (inner_end_ == 0 ? kBucketSize - 1 : inner_end_ - 1);
      size_--;
      throw;
    }
  }
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& item) {
  emplace_back(std::move(item));
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& item) {
  emplace_back(item);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  (outer_[outer_end_] + inner_end_)->~T();
  if (inner_end_ == 0) {
    inner_end_ = kBucketSize - 1;
    outer_end_--;
    outer_size_--;
  } else {
    inner_end_--;
  }
  size_--;
}

template <typename T, typename Allocator>
template <class... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (outer_start_ == 0 && inner_start_ == 0) {
    reserve();
  }
  if (inner_start_ == 0) {
    if (outer_start_ == memory_outer_start_) {
      outer_[outer_start_ - 1] = alloc_traits::allocate(alloc_, kBucketSize);
      memory_outer_start_--;
    }
    outer_start_--;
    outer_size_++;
    inner_start_ = kBucketSize - 1;
  } else {
    inner_start_--;
  }
  size_++;
  try {
    alloc_traits::construct(alloc_, outer_[outer_start_] + inner_start_,
                            std::forward<Args>(args)...);
  } catch (...) {
    size_--;
    outer_start_++;
    if (inner_start_ == kBucketSize - 1) {
      inner_start_ = 0;
      outer_size_--;
    } else {
      inner_start_++;
    }
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& elem) {
  emplace_front(elem);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& elem) {
  emplace_front(std::move(elem));
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  (outer_[outer_start_] + inner_start_)->~T();
  if (inner_start_ == kBucketSize - 1) {
    inner_start_ = 0;
    outer_start_++;
    outer_size_--;
  } else {
    inner_start_++;
  }
  size_--;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>::Iterator(T** outer_pointer,
                                                 size_t out_ind, size_t in_ind)
    : outer_(outer_pointer), outer_index_(out_ind), inner_index_(in_ind) {}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>&
Deque<T, Allocator>::Iterator<IsConst>::operator+=(int n) {
  if (outer_ == nullptr) {
    return *this;
  }
  n = n - (kBucketSize - inner_index_ - 1);
  int outer_change = (n + kBucketSize - 1) / kBucketSize;
  inner_index_ = (n + kBucketSize - 1) % kBucketSize;
  outer_index_ += outer_change;
  return *this;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>&
Deque<T, Allocator>::Iterator<IsConst>::operator-=(int n) {
  if (outer_ == nullptr) {
    return *this;
  }
  n = n - inner_index_;
  int outer_change = (n + kBucketSize - 1) / kBucketSize;
  inner_index_ = (kBucketSize - n % kBucketSize) % kBucketSize;
  outer_index_ -= outer_change;
  return *this;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator-(int n) const {
  auto copy = *this;
  copy -= n;
  return copy;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator+(int n) const {
  auto copy = *this;
  copy += n;
  return copy;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator++(int) {
  auto copy(*this);
  operator++();
  return copy;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>&
Deque<T, Allocator>::Iterator<IsConst>::operator++() {
  if (inner_index_ == kBucketSize - 1) {
    outer_index_++;
    inner_index_ = 0;
  } else {
    inner_index_++;
  }
  return *this;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator--(int) {
  auto copy(*this);
  operator--();
  return copy;
}
template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::Iterator<IsConst>
Deque<T, Allocator>::Iterator<IsConst>::operator--() {
  if (inner_index_ == 0) {
    outer_index_--;
    inner_index_ = kBucketSize - 1;
  } else {
    inner_index_--;
  }
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::iterator Deque<T, Allocator>::begin() {
  if (outer_ == nullptr) {
    return iterator(outer_, 0, 0);
  }
  return iterator(outer_, outer_start_, inner_start_);
}
template <typename T, typename Allocator>
Deque<T, Allocator>::iterator Deque<T, Allocator>::end() {
  if (outer_ == nullptr) {
    return Iterator<false>(outer_, 0, 0);
  }
  return Iterator<false>(outer_, outer_end_ + (inner_end_ + 1) / kBucketSize,
                         (inner_end_ + 1) % kBucketSize);
}
template <typename T, typename Allocator>
Deque<T, Allocator>::const_iterator Deque<T, Allocator>::cend() {
  if (outer_ == nullptr) {
    return Iterator<true>(outer_, 0, 0);
  }
  return Iterator<true>(outer_, outer_end_ + (inner_end_ + 1) / kBucketSize,
                        (inner_end_ + 1) % kBucketSize);
}
template <typename T, typename Allocator>
Deque<T, Allocator>::const_iterator Deque<T, Allocator>::cbegin() {
  if (outer_ == nullptr) {
    return Iterator<true>(outer_, 0, 0);
  }
  return const_iterator(outer_, outer_start_, inner_start_);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(Deque<T, Allocator>::iterator it1,
                                 const T& item) {
  if (it1 != this->end()) {
    T last = outer_[outer_end_][inner_end_];
    outer_[outer_end_][inner_end_] = item;
    auto it_last = this->end() - 1;
    while (it_last != it1) {
      std::swap(*it_last, *(it_last - 1));
      --it_last;
    }
    this->push_back(last);
  } else {
    this->push_back(item);
  }
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(Deque<T, Allocator>::iterator iter, T&& item) {
  if (iter != this->end()) {
    T last = outer_[outer_end_][inner_end_];
    outer_[outer_end_][inner_end_] = item;
    auto it_last = this->end() - 1;
    while (it_last != iter) {
      std::swap(*it_last, *(it_last - 1));
      --it_last;
    }
    this->push_back(last);
  } else {
    this->push_back(item);
  }
  item.~T();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(Deque<T, Allocator>::iterator it1) {
  while (it1 != this->end() - 1) {
    ++it1;
    std::swap(*it1, *(it1 - 1));
  }
  this->pop_back();
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::reserve() {
  size_t new_outer_capacity = outer_capacity_ * 2;
  size_t new_outer_start = new_outer_capacity / 4;
  size_t new_outer_end = new_outer_start + outer_size_ - 1;
  size_t new_memory_outer_start =
      memory_outer_start_ + new_outer_start - outer_start_;
  size_t new_memory_outer_end = memory_outer_end_ + new_outer_end - outer_end_;
  T** new_outer =
      bucket_alloc_traits::allocate(outer_alloc_, new_outer_capacity);
  for (size_t out_ind = 0; out_ind < outer_size_; ++out_ind) {
    new_outer[out_ind + new_memory_outer_start] =
        outer_[out_ind + memory_outer_start_];
  }
  bucket_alloc_traits::deallocate(outer_alloc_, outer_, outer_capacity_);
  outer_ = new_outer;
  outer_capacity_ = new_outer_capacity;
  outer_start_ = new_outer_start;
  outer_end_ = new_outer_end;
  memory_outer_end_ = new_memory_outer_end;
  memory_outer_start_ = new_memory_outer_start;
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::swap(Deque<T, Allocator>& first,
                               Deque<T, Allocator>& second) {
  std::swap(first.outer_, second.outer_);
  std::swap(first.outer_end_, second.outer_end_);
  std::swap(first.outer_start_, second.outer_start_);
  std::swap(first.inner_start_, second.inner_start_);
  std::swap(first.inner_end_, second.inner_end_);
  std::swap(first.outer_size_, second.outer_size_);
  std::swap(first.size_, second.size_);
  std::swap(first.outer_capacity_, second.outer_capacity_);
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::initialize(Deque<T, Allocator>& deque,
                                     const Deque<T, Allocator>& other) {
  deque.outer_size_ = other.outer_size_;
  deque.outer_capacity_ = other.outer_capacity_;
  deque.outer_start_ = other.outer_start_;
  deque.outer_end_ = other.outer_end_;
  deque.inner_end_ = other.inner_end_;
  deque.inner_start_ = other.inner_start_;
  deque.size_ = other.size_;
  deque.memory_outer_end_ = other.memory_outer_end_;
  deque.memory_outer_start_ = other.memory_outer_start_;
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::destroy(Deque& tmp, size_t out_ind, size_t in_ind,
                                  size_t fin_ind) {
  if (tmp.outer_end_ != tmp.outer_start_ and out_ind > 0) {
    for (size_t i = tmp.outer_start_ + 1; i < out_ind; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        alloc_traits::destroy(tmp.alloc_, tmp.outer_[i] + j);
      }
    }
    for (size_t i = 0; i < in_ind; ++i) {
      alloc_traits::destroy(tmp.alloc_, tmp.outer_[out_ind] + i);
    }
    if (out_ind == tmp.outer_end_) {
      for (size_t i = 0; i < fin_ind; ++i) {
        alloc_traits::destroy(tmp.alloc_, tmp.outer_[tmp.outer_end_] + i);
      }
    }
  }
  for (size_t i = tmp.outer_start_; i <= tmp.outer_end_; ++i) {
    alloc_traits::deallocate(tmp.alloc_, tmp.outer_[i], kBucketSize);
  }
}
template <typename T, typename Allocator>
void Deque<T, Allocator>::constructor_destroy(size_t out_ind, size_t in_ind,
                                              size_t index) {
  for (size_t i = 0; i < out_ind; ++i) {
    for (size_t j = 0; j < kBucketSize; ++j) {
      alloc_traits::destroy(alloc_, outer_[outer_start_ + i] + j);
    }
  }
  for (size_t i = 0; i < index; ++i) {
    alloc_traits::destroy(alloc_, outer_[outer_end_] + i);
  }
  if (index == 0 && in_ind != kBucketSize) {
    for (size_t i = 0; i < in_ind; ++i) {
      alloc_traits::destroy(alloc_, outer_[outer_start_ + out_ind] + i);
    }
  }
  for (size_t i = 0; i < outer_size_; ++i) {
    alloc_traits::deallocate(alloc_, outer_[outer_start_ + i], kBucketSize);
  }
  bucket_alloc_traits::deallocate(outer_alloc_, outer_, outer_capacity_);
}
