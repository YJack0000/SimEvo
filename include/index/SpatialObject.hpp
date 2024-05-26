#ifndef SPATIAL_OBJECT_HPP
#define SPATIAL_OBJECT_HPP

#include <utility>

template <typename T> class SpatialObject {
public:
  SpatialObject(const T &object, float x, float y)
      : object(object), position(std::make_pair(x, y)) {}

    T getObject() const { return object; }
  std::pair<int, int> getPosition() const { return position; }
  void setPosition(int x, int y) { position = std::make_pair(x, y); }

private:
  T object;
  std::pair<int, int> position;
};

#endif
