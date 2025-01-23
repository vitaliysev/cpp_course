#include <iostream>
template <typename T, typename Allocator = std::allocator<T>>
class List {
 public:
  using value_type = T;
  using allocator_type = Allocator;
  List() = default;
  List(size_t count, const value_type& value,
       const Allocator& alloc = Allocator());
  explicit List(size_t count, const Allocator& alloc = Allocator());
  List(const List& other);
  List(std::initializer_list<value_type> init,
       const Allocator& alloc = Allocator());
  List& operator=(const List& other);
  ~List();
  Allocator get_allocator() const { return alloc_; }
  struct BaseNode;
  struct Node;
  template <bool IsConst>
  class Iterator;
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<Iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<Iterator<true>>;
  iterator begin() const { return iterator(start_.next); }
  iterator end() const { return iterator(start_.next->prev); }
  const_iterator cbegin() { return const_iterator(start_.next); }
  const_iterator cend() { return const_iterator(&start_); }
  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
  value_type& front() { return start_.next->value; }
  const value_type& front() const { return start_.next->value; }
  value_type& back() { return start_.prev->value; }
  const value_type& back() const { return start_.prev->value; }
  bool empty() { return size_ == 0; }
  size_t size() const { return size_; }
  void push_back(const value_type& value);
  void pop_back();
  void push_front(const value_type& value);
  void pop_front();

 private:
  void help(const List& other);
  using alloc_traits = std::allocator_traits<Allocator>;
  using node_alloc = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = typename alloc_traits::template rebind_traits<Node>;
  node_alloc alloc_;
  BaseNode start_;
  size_t size_ = 0;
};
template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::Iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  Iterator(BaseNode* const kNode) : cur_node_(kNode){};
  reference operator*() const { return static_cast<Node*>(cur_node_)->value; };
  pointer operator->() const { return &static_cast<Node*>(cur_node_)->value; };
  Iterator operator++(int) {
    auto copy(*this);
    operator++();
    return copy;
  }
  Iterator& operator++() {
    cur_node_ = cur_node_->next;
    return *this;
  }
  Iterator operator--(int) {
    auto copy(*this);
    operator++();
    return copy;
  }
  Iterator& operator--() {
    cur_node_ = cur_node_->prev;
    return *this;
  }
  bool operator==(const Iterator& other) {
    return cur_node_ == other.cur_node_;
  }
  bool operator!=(const Iterator& other) { return !(*this == other); }

 private:
  BaseNode* cur_node_ = nullptr;
};
template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const value_type& value,
                         const Allocator& alloc)
    : alloc_(alloc), size_(count) {
  BaseNode* cur_node = &start_;
  size_t index = 0;
  try {
    for (; index < count; ++index) {
      cur_node->next = node_alloc_traits::allocate(alloc_, 1);
      try {
        node_alloc_traits::construct(alloc_, static_cast<Node*>(cur_node->next),
                                     value);
      } catch (...) {
        node_alloc_traits::deallocate(alloc_,
                                      static_cast<Node*>(cur_node->next), 1);
        throw;
      }
      cur_node->next->prev = cur_node;
      cur_node = cur_node->next;
    }
    cur_node->next = &start_;
    start_.prev = cur_node;
  } catch (...) {
    for (size_t j = 0; j < index; ++j) {
      cur_node = cur_node->prev;
      node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node->next));
      node_alloc_traits::deallocate(alloc_, static_cast<Node*>(cur_node->next),
                                    1);
    }
    size_ = 0;
    throw;
  }
}
template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc)
    : alloc_(alloc), size_(count) {
  BaseNode* cur_node = &start_;
  size_t index = 0;
  try {
    for (; index < size_; ++index) {
      cur_node->next = node_alloc_traits::allocate(alloc_, 1);
      try {
        node_alloc_traits::construct(alloc_,
                                     static_cast<Node*>(cur_node->next));
      } catch (...) {
        node_alloc_traits::deallocate(alloc_,
                                      static_cast<Node*>(cur_node->next), 1);
        throw;
      }
      cur_node->next->prev = cur_node;
      cur_node = cur_node->next;
    }
    cur_node->next = &start_;
    start_.prev = cur_node;
  } catch (...) {
    for (size_t j = 0; j < index; ++j) {
      cur_node = cur_node->prev;
      node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node->next));
      node_alloc_traits::deallocate(alloc_, static_cast<Node*>(cur_node->next),
                                    1);
    }
    size_ = 0;
    throw;
  }
}
template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : size_(other.size_),
      alloc_(node_alloc_traits::select_on_container_copy_construction(
          other.alloc_)) {
  BaseNode* cur_node = &start_;
  const BaseNode* other_node = &other.start_;
  size_t index = 0;
  try {
    for (; index < other.size_; ++index) {
      cur_node->next = node_alloc_traits::allocate(alloc_, 1);
      try {
        node_alloc_traits::construct(
            alloc_, static_cast<Node*>(cur_node->next),
            static_cast<const Node*>(other_node->next)->value);
      } catch (...) {
        node_alloc_traits::deallocate(alloc_,
                                      static_cast<Node*>(cur_node->next), 1);
        throw;
      }
      cur_node->next->prev = cur_node;
      cur_node = cur_node->next;
      other_node = other_node->next;
    }
    cur_node->next = &start_;
    start_.prev = cur_node;
  } catch (...) {
    for (size_t j = 0; j < index; ++j) {
      cur_node = cur_node->prev;
      node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node->next));
      node_alloc_traits::deallocate(alloc_, static_cast<Node*>(cur_node->next),
                                    1);
    }
    throw;
  }
};
template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<value_type> init,
                         const Allocator& alloc)
    : alloc_(alloc), size_(init.size()) {
  BaseNode* cur_node = &start_;
  for (const auto& elem : init) {
    cur_node->next = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, static_cast<Node*>(cur_node->next),
                                 elem);
    cur_node->next->prev = cur_node;
    cur_node = cur_node->next;
  }
  cur_node->next = &start_;
  start_.prev = cur_node;
}
template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (&other == this) {
    return *this;
  }
  node_alloc real_alloc = alloc_;
  if (node_alloc_traits::propagate_on_container_copy_assignment::value) {
    real_alloc = other.alloc_;
    BaseNode* cur_node = start_.prev;
    const BaseNode* other_node = &other.start_;
    size_t index = 0;
    try {
      for (; index < other.size_; ++index) {
        cur_node->next = node_alloc_traits::allocate(real_alloc, 1);
        try {
          node_alloc_traits::construct(
              real_alloc, static_cast<Node*>(cur_node->next),
              static_cast<const Node*>(other_node->next)->value);
        } catch (...) {
          node_alloc_traits::deallocate(real_alloc,
                                        static_cast<Node*>(cur_node->next), 1);
          throw;
        }
        cur_node->next->prev = cur_node;
        cur_node = cur_node->next;
        other_node = other_node->next;
      }
      cur_node->next = &start_;
      BaseNode* cur_node2 = start_.prev->prev;
      BaseNode* real_start = start_.prev->next;
      start_.prev = cur_node;
      for (size_t i = 0; i < size_; ++i) {
        node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node2->next));
        node_alloc_traits::deallocate(alloc_,
                                      static_cast<Node*>(cur_node2->next), 1);
        cur_node2 = cur_node2->prev;
      }
      start_.next = real_start;
      real_start->prev = &start_;
      alloc_ = real_alloc;
      size_ = other.size_;
    } catch (...) {
      for (size_t j = 0; j < index; ++j) {
        cur_node = cur_node->prev;
        node_alloc_traits::destroy(real_alloc,
                                   static_cast<Node*>(cur_node->next));
        node_alloc_traits::deallocate(real_alloc,
                                      static_cast<Node*>(cur_node->next), 1);
      }
      cur_node->next = &start_;
      throw;
    }
  } else {
    help(other);
  }
  return *this;
}
template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  BaseNode* cur_node = start_.prev;
  for (size_t i = 0; i < size_; ++i) {
    cur_node = cur_node->prev;
    node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node->next));
    node_alloc_traits::deallocate(alloc_, static_cast<Node*>(cur_node->next),
                                  1);
  }
}
template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const value_type& value) {
  ++size_;
  BaseNode* last_node = start_.prev;
  try {
    last_node->next = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, static_cast<Node*>(last_node->next),
                                   value);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, static_cast<Node*>(last_node->next),
                                    1);
      throw;
    }
    BaseNode* real_last_node = last_node->next;
    real_last_node->prev = last_node;
    real_last_node->next = &start_;
    start_.prev = real_last_node;
  } catch (...) {
    --size_;
    throw;
  }
}
template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  --size_;
  BaseNode* real_last_node = start_.prev->prev;
  node_alloc_traits::destroy(alloc_, static_cast<Node*>(real_last_node->next));
  node_alloc_traits::deallocate(alloc_,
                                static_cast<Node*>(real_last_node->next), 1);
  real_last_node->next = &start_;
  start_.prev = real_last_node;
}
template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const value_type& value) {
  ++size_;
  BaseNode* first_node = start_.next;
  try {
    first_node->prev = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, static_cast<Node*>(first_node->prev),
                                   value);
    } catch (...) {
      node_alloc_traits::construct(alloc_, static_cast<Node*>(first_node->prev),
                                   value);
    }
    BaseNode* real_first_node = first_node->prev;
    real_first_node->next = first_node;
    real_first_node->prev = &start_;
    start_.next = real_first_node;
  } catch (...) {
    --size_;
    throw;
  }
}
template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  --size_;
  BaseNode* real_first_node = start_.next->next;
  node_alloc_traits::destroy(alloc_, static_cast<Node*>(real_first_node->prev));
  node_alloc_traits::deallocate(alloc_,
                                static_cast<Node*>(real_first_node->prev), 1);
  real_first_node->prev = &start_;
  start_.next = real_first_node;
}
template <typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
  BaseNode() = default;
  BaseNode* prev = this;
  BaseNode* next = this;
};
template <typename T, typename Allocator>
struct List<T, Allocator>::Node : List<T, Allocator>::BaseNode {
  Node(const value_type& item) : value(item) {}
  Node() = default;
  value_type value;
};
template <typename T, typename Allocator>
void List<T, Allocator>::help(const List& other) {
  size_t real_size = std::min(size_, other.size_);
  BaseNode* cur_node = &start_;
  const BaseNode* other_node = &other.start_;
  for (size_t i = 0; i < real_size; ++i) {
    static_cast<Node*>(cur_node->next)->value =
        static_cast<const Node*>(other_node->next)->value;
    cur_node = cur_node->next;
    other_node = other_node->next;
  }
  if (size_ < other.size_) {
    for (size_t i = size_; i < other.size_; ++i) {
      cur_node->next = node_alloc_traits::allocate(alloc_, 1);
      node_alloc_traits::construct(
          alloc_, static_cast<Node*>(cur_node->next),
          static_cast<const Node*>(other_node->next)->value);
      cur_node->next->prev = cur_node;
      cur_node = cur_node->next;
      other_node = other_node->next;
    }
  } else {
    BaseNode* cur_node = start_.prev;
    for (size_t j = other.size_; j < size_; ++j) {
      cur_node = cur_node->prev;
      node_alloc_traits::destroy(alloc_, static_cast<Node*>(cur_node->next));
      node_alloc_traits::deallocate(alloc_, static_cast<Node*>(cur_node->next),
                                    1);
    }
  }
  start_.prev = cur_node;
  cur_node->next = &start_;
  size_ = other.size_;
}
