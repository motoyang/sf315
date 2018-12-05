#pragma once

class RingBuffer {
public:
  RingBuffer(size_t capacity)
      : beg_index_(0), end_index_(0), size_(0), capacity_(capacity) {
    data_ = new char[capacity];
  }

  ~RingBuffer() { delete[] data_; }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  // Return number of bytes written.
  size_t write(const char *data, size_t bytes) {
    if ((bytes == 0) || (bytes > (capacity_ - size_))) {
      return 0;
    }

    size_t capacity = capacity_;
    size_t bytes_to_write = bytes;

    // Write in a single step
    if (bytes_to_write <= capacity - end_index_) {
      memcpy(data_ + end_index_, data, bytes_to_write);
      end_index_ += bytes_to_write;
      if (end_index_ == capacity)
        end_index_ = 0;
    }
    // Write in two steps
    else {
      size_t size_1 = capacity - end_index_;
      memcpy(data_ + end_index_, data, size_1);
      size_t size_2 = bytes_to_write - size_1;
      memcpy(data_, data + size_1, size_2);
      end_index_ = size_2;
    }

    size_ += bytes_to_write;
    return bytes_to_write;
  }

  // Return number of bytes read.
  size_t read(char *data, size_t bytes) {
    if ((bytes == 0) || (bytes > size_)) {
      return 0;
    }

    size_t capacity = capacity_;
    size_t bytes_to_read = bytes;

    // Read in a single step
    if (bytes_to_read <= capacity - beg_index_) {
      memcpy(data, data_ + beg_index_, bytes_to_read);
      beg_index_ += bytes_to_read;
      if (beg_index_ == capacity)
        beg_index_ = 0;
    }
    // Read in two steps
    else {
      size_t size_1 = capacity - beg_index_;
      memcpy(data, data_ + beg_index_, size_1);
      size_t size_2 = bytes_to_read - size_1;
      memcpy(data + size_1, data_, size_2);
      beg_index_ = size_2;
    }

    size_ -= bytes_to_read;
    return bytes_to_read;
  }

  // return nullptr means error, otherwise return the pointer of the first 'c'.
  char *search(char c) const {
    if (size_ == 0) {
      return nullptr;
    }

    char *r = nullptr;
    size_t capacity = capacity_;
    size_t bytes_to_search = size_;

    // search in a single step
    if (bytes_to_search <= capacity - beg_index_) {
      r = (char *)memchr(data_ + beg_index_, c, bytes_to_search);
    }
    // search in two steps
    else {
      size_t size_1 = capacity - beg_index_;
      r = (char *)memchr(data_ + beg_index_, c, size_1);
      if (!r) {
        size_t size_2 = bytes_to_search - size_1;
        r = (char *)memchr(data_, c, size_2);
      }
    }
    return r;
  }

  // return -1 means error, otherwise return 0.
  int advance(size_t bytes) {
    if (bytes > size_) {
      return -1;
    }

    beg_index_ += bytes;
    if (beg_index_ >= capacity_) {
      beg_index_ -= capacity_;
    }
    size_ -= bytes;

    return 0;
  }

  // return -1 means errors.
  int offset(const char *p) const {
    int r = -1;
    if (p < data_ || p >= data_ + capacity_) {
      return r;
    }

    if (p >= (data_ + beg_index_)) {
      r = p - (data_ + beg_index_);
    } else {
      r = p + capacity_ - (data_ + beg_index_);
    }

    if (r > size_) {
      return -1;
    }
    return r;
  }

  // Return nullptr means error, otherwise return the data address.
  char *peek(char *data, size_t bytes) const {
    if (bytes > size_) {
      return nullptr;
    }

    size_t capacity = capacity_;
    size_t bytes_to_peek = bytes;

    // peek in a single step
    if (bytes_to_peek <= capacity - beg_index_) {
      memcpy(data, data_ + beg_index_, bytes_to_peek);
    }
    // peek in two steps
    else {
      size_t size_1 = capacity - beg_index_;
      memcpy(data, data_ + beg_index_, size_1);
      size_t size_2 = bytes_to_peek - size_1;
      memcpy(data + size_1, data_, size_2);
    }

    return data;
  }

private:
  size_t beg_index_, end_index_, size_, capacity_;
  char *data_;
};
