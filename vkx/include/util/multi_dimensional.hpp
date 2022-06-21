#pragma once

namespace vkx
{
  template <class T>
  class Matrix : public std::vector<T>
  {
  public:
    using std::vector<T>::vector;

    virtual ~Matrix() = default;

    void set(std::int32_t i, T const &data)
    {
      std::vector<T>::operator[](i) = data;
    }

    virtual std::int32_t getDimensionSize(std::int32_t slice) const = 0;
  };

  template <class T>
  class Matrix2D : public Matrix<T>
  {
  public:
    Matrix2D() = default;

    explicit Matrix2D(std::int32_t width, std::int32_t height)
        : Matrix<T>(width * height),
          width(width),
          height(height) {}

    explicit Matrix2D(std::int32_t size)
        : Matrix2D(size, size) {}

    [[nodiscard]] T const &at(std::int32_t x, std::int32_t y) const
    {
      return std::vector<T>::operator[](index(x, y));
    }

    void set(std::int32_t x, std::int32_t y, T const &data)
    {
      std::vector<T>::operator[](index(x, y)) = data;
    }

    [[nodiscard]] std::int32_t getDimensionSize(std::int32_t slice) const override
    {
      switch (slice)
      {
      case 0:
        return width;
      case 1:
        return height;
      default:
        throw std::out_of_range("Slice out of range.");
      }
    }

    constexpr auto index(std::int32_t x, std::int32_t y) const
    {
      return x + width * y;
    }

    [[nodiscard]] auto getWidth() const
    {
      return width;
    }

    [[nodiscard]] auto getHeight() const
    {
      return height;
    }

  protected:
    std::int32_t const width;
    std::int32_t const height;

    constexpr auto validLocation(std::int32_t x, std::int32_t y) const
    {
      return x >= 0 && x < width && y >= 0 && y < height;
    }
  };

  template <class T>
  class Matrix3D : public Matrix<T>
  {
  public:
    Matrix3D() = default;

    explicit Matrix3D(std::int32_t width, std::int32_t height, std::int32_t depth)
        : Matrix<T>(width * height * depth),
          width(width),
          height(height),
          depth(depth) {}

    explicit Matrix3D(std::int32_t size)
        : Matrix3D(size, size, size) {}

    [[nodiscard]] T const &at(std::int32_t x, std::int32_t y, std::int32_t z) const
    {
      return std::vector<T>::operator[](index(x, y, z));
    }

    void set(std::int32_t x, std::int32_t y, std::int32_t z, T const &data)
    {
      std::vector<T>::operator[](index(x, y, z)) = data;
    }

    [[nodiscard]] std::int32_t getDimensionSize(std::int32_t slice) const override
    {
      switch (slice)
      {
      case 0:
        return width;
      case 1:
        return height;
      case 2:
        return depth;
      default:
        throw std::out_of_range("Slice out of range.");
      }
    }

    constexpr auto index(std::int32_t x, std::int32_t y, std::int32_t z) const
    {
      return z * width * height + y * width + x;
    }

    [[nodiscard]] auto getWidth() const
    {
      return width;
    }

    [[nodiscard]] auto getHeight() const
    {
      return height;
    }

    [[nodiscard]] auto getDepth() const
    {
      return depth;
    }

  protected:
    std::int32_t const width;
    std::int32_t const height;
    std::int32_t const depth;

    constexpr auto validLocation(std::int32_t x, std::int32_t y, std::int32_t z) const
    {
      return x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth;
    }
  };
}