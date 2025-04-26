#pragma once

#include <cstdlib>          // 用于 exit
#include <cstring>          // 用于 memcpy
#include <initializer_list> // 用于初始化列表
#include <iostream>         // 用于输入输出
#include <new>              // 用于 placement new
#include <utility>          // 用于 std::move, std::swap

namespace nexus {

// 定义 vector 类
template <typename T> class vector {
public:
  using value_type = T;
  using size_type = size_t;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using iterator = T *;
  using const_iterator = const T *;

  // 默认构造函数
  vector() : first(nullptr), last(nullptr), end_ptr(nullptr) {}

  // 构造 n 个元素，初值为 value（默认T()）
  vector(size_type count, const T &value = T())
      : first(new T[count]), last(first + count), end_ptr(first + count) {
    for (size_type i = 0; i < count; ++i) {
      first[i] = value;
    }
  }

  // 使用初始化列表构造
  vector(std::initializer_list<T> init) : vector(init.size(), T()) {
    size_type i = 0;
    for (const auto &elem : init) {
      first[i++] = elem;
    }
  }

  // 拷贝构造函数
  vector(const vector &other)
      : first(new T[other.capacity()]), last(first + other.size()),
        end_ptr(first + other.capacity()) {
    for (size_type i = 0; i < other.size(); ++i) {
      first[i] = other.first[i];
    }
  }

  // 移动构造函数
  vector(vector &&other) noexcept
      : first(other.first), last(other.last), end_ptr(other.end_ptr) {
    other.first = nullptr;
    other.last = nullptr;
    other.end_ptr = nullptr;
  }

  // 从指针区间构造
  vector(pointer start, pointer end)
      : first(new T[end - start]), last(first + (end - start)),
        end_ptr(first + (end - start)) {
    for (size_type i = 0; i < static_cast<size_type>(end - start); ++i) {
      first[i] = start[i];
    }
  }

  // 析构函数
  ~vector() { delete[] first; }

  // 拷贝赋值
  vector &operator=(const vector &other) {
    if (this != &other) {
      vector temp(other);
      swap(temp);
    }
    return *this;
  }

  // 移动赋值
  vector &operator=(vector &&other) noexcept {
    if (this != &other) {
      delete[] first;
      first = other.first;
      last = other.last;
      end_ptr = other.end_ptr;
      other.first = nullptr;
      other.last = nullptr;
      other.end_ptr = nullptr;
    }
    return *this;
  }

  // 下标访问
  reference operator[](size_type index) { return first[index]; }
  const_reference operator[](size_type index) const { return first[index]; }

  // 带边界检查的访问
  reference at(size_type index) {
    if (index >= size()) {
      std::cerr << "索引越界: " << index << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return first[index];
  }
  const_reference at(size_type index) const {
    if (index >= size()) {
      std::cerr << "索引越界: " << index << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return first[index];
  }

  // 获取第一个元素
  reference front() {
    if (empty()) {
      std::cerr << "vector为空，无法front()" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return *first;
  }
  const_reference front() const {
    if (empty()) {
      std::cerr << "vector为空，无法front()" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return *first;
  }

  // 获取最后一个元素
  reference back() {
    if (empty()) {
      std::cerr << "vector为空，无法back()" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return *(last - 1);
  }
  const_reference back() const {
    if (empty()) {
      std::cerr << "vector为空，无法back()" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return *(last - 1);
  }

  // 获取指向开始的迭代器
  iterator begin() noexcept { return first; }
  const_iterator begin() const noexcept { return first; }
  const_iterator cbegin() const noexcept { return first; }

  // 获取指向结束的迭代器
  iterator end() noexcept { return last; }
  const_iterator end() const noexcept { return last; }
  const_iterator cend() const noexcept { return last; }

  // 当前元素数量
  size_type size() const noexcept {
    return static_cast<size_type>(last - first);
  }

  // 当前容量
  size_type capacity() const noexcept {
    return static_cast<size_type>(end_ptr - first);
  }

  // 是否为空
  bool empty() const noexcept { return size() == 0; }

  // 插入元素（拷贝）
  void push_back(const T &value) {
    if (last == end_ptr) {
      grow();
    }
    *last++ = value;
  }

  // 插入元素（移动）
  void push_back(T &&value) {
    if (last == end_ptr) {
      grow();
    }
    *last++ = std::move(value);
  }

  // 原地构造元素
  template <typename... Args> void emplace_back(Args &&...args) {
    if (last == end_ptr) {
      grow();
    }
    new (last++) T(std::forward<Args>(args)...);
  }

  // 删除最后一个元素
  void pop_back() {
    if (empty()) {
      std::cerr << "vector为空，无法pop_back" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    --last;
    last->~T();
  }

  // 清空所有元素
  void clear() noexcept {
    while (last != first) {
      (--last)->~T();
    }
  }

  // 调整容器大小
  void resize(size_type new_size, const T &value = T()) {
    if (new_size < size()) {
      while (size() > new_size) {
        pop_back();
      }
    } else if (new_size > size()) {
      reserve(new_size);
      while (size() < new_size) {
        push_back(value);
      }
    }
  }

  // 预留空间
  void reserve(size_type new_cap) {
    if (new_cap > capacity()) {
      reallocate(new_cap);
    }
  }

  // 减小容量到刚好
  void shrink_to_fit() {
    if (capacity() > size()) {
      reallocate(size());
    }
  }

  // 交换
  void swap(vector &other) noexcept {
    std::swap(first, other.first);
    std::swap(last, other.last);
    std::swap(end_ptr, other.end_ptr);
  }

  // 支持 std::cin >> arr
  friend std::istream &operator>>(std::istream &is, vector &vec) {
    for (auto &elem : vec) {
      is >> elem;
    }
    return is;
  }

  // 支持 std::cout << arr
  friend std::ostream &operator<<(std::ostream &os, const vector &vec) {
    for (size_type i = 0; i < vec.size(); ++i) {
      os << vec[i];
      if (i + 1 != vec.size())
        os << ' ';
    }
    os << '\n'; // 输出后换行
    return os;
  }

  // 相等
  friend bool operator==(const vector &lhs, const vector &rhs) {
    if (lhs.size() != rhs.size())
      return false;
    for (size_type i = 0; i < lhs.size(); ++i) {
      if (!(lhs[i] == rhs[i]))
        return false;
    }
    return true;
  }

  // 不相等
  friend bool operator!=(const vector &lhs, const vector &rhs) {
    return !(lhs == rhs);
  }

  // 小于（字典序）
  friend bool operator<(const vector &lhs, const vector &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }

  // 小于等于
  friend bool operator<=(const vector &lhs, const vector &rhs) {
    return !(rhs < lhs);
  }

  // 大于
  friend bool operator>(const vector &lhs, const vector &rhs) {
    return rhs < lhs;
  }

  // 大于等于
  friend bool operator>=(const vector &lhs, const vector &rhs) {
    return !(lhs < rhs);
  }

private:
  pointer first;
  pointer last;
  pointer end_ptr;

  // 扩容（容量翻倍）
  void grow() {
    size_type old_cap = capacity();
    size_type new_cap = old_cap == 0 ? 1 : old_cap * 2;
    reallocate(new_cap);
  }

  // 重新分配空间
  void reallocate(size_type new_cap) {
    pointer new_data = new T[new_cap];
    size_type old_size = size();

    for (size_type i = 0; i < old_size; ++i) {
      new_data[i] = std::move(first[i]);
    }

    delete[] first;
    first = new_data;
    last = first + old_size;
    end_ptr = first + new_cap;
  }
};

} // namespace nexus
