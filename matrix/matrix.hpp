#include <assert.h>

#include <iostream>
#include <vector>
template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix();
  Matrix(const T& elem);
  Matrix(std::vector<std::vector<T>>& old_vector);
  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& other);
  Matrix<N, M, T>& operator-=(Matrix<N, M, T>& other);
  Matrix<N, M, T> operator+(const Matrix<N, M, T>& other);
  Matrix<N, M, T> operator-(Matrix<N, M, T>& other);
  Matrix<N, M, T> operator*(const T& multiplier);
  template <size_t P>
  Matrix<N, P, T> operator*(const Matrix<M, P, T>& other);
  Matrix<M, N, T> Transposed();
  T Trace();
  const T& operator()(size_t i, size_t ind) const;
  T& operator()(size_t i, size_t ind);
  bool operator==(Matrix<N, M, T>& other);

 private:
  std::vector<std::vector<T>> data_;
};
template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix()
    : data_(std::vector<std::vector<T>>(N, std::vector<T>(M))) {}
template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(const T& elem)
    : data_(std::vector<std::vector<T>>(N, std::vector<T>(M, elem))) {}
template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(std::vector<std::vector<T>>& old_vector)
    : data_(old_vector) {}
template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& other) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < M; ++ind) {
      data_[i][ind] += other(i, ind);
    }
  }
  return *this;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(Matrix<N, M, T>& other) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < M; ++ind) {
      data_[i][ind] -= other(i, ind);
    }
  }
  return *this;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator+(const Matrix<N, M, T>& other) {
  Matrix<N, M, T> newmatrix(*this);
  newmatrix += other;
  return newmatrix;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator-(Matrix<N, M, T>& other) {
  Matrix<N, M, T> newmatrix(*this);
  newmatrix -= other;
  return newmatrix;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> Matrix<N, M, T>::operator*(const T& multiplier) {
  Matrix<N, M, T> newmatrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < M; ++ind) {
      newmatrix(i, ind) = data_[i][ind] * multiplier;
    }
  }
  return newmatrix;
}
template <size_t N, size_t M, typename T>
template <size_t P>
Matrix<N, P, T> Matrix<N, M, T>::operator*(const Matrix<M, P, T>& other) {
  Matrix<N, P, T> newmatrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < P; ++ind) {
      T sum = 0.0;
      for (size_t z = 0; z < M; ++z) {
        sum += (data_[i][z] * other(z, ind));
      }
      newmatrix(i, ind) = sum;
    }
  }
  return newmatrix;
}
template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> newmatrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < M; ++ind) {
      newmatrix(ind, i) = data_[i][ind];
    }
  }
  return newmatrix;
}
template <size_t N, size_t M, typename T>
T Matrix<N, M, T>::Trace() {
  static_assert(N == M);
  T sum = data_[0][0];
  for (size_t i = 1; i < N; ++i) {
    sum += data_[i][i];
  }
  return sum;
}
template <size_t N, size_t M, typename T>
const T& Matrix<N, M, T>::operator()(size_t i, size_t ind) const {
  return data_[i][ind];
}
template <size_t N, size_t M, typename T>
T& Matrix<N, M, T>::operator()(size_t i, size_t ind) {
  return data_[i][ind];
}
template <size_t N, size_t M, typename T>
bool Matrix<N, M, T>::operator==(Matrix<N, M, T>& other) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t ind = 0; ind < M; ++ind) {
      if (data_[i][ind] != other(i, ind)) {
        return false;
      }
    }
  }
  return true;
}