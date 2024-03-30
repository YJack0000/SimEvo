#include "Environment.hpp"
#include "ISpatialIndex.hpp"
#include <string>

Environment::Environment(int width, int height, const std::string &type)
    : width(width), height(height) {
  if (type == std::string("optimize")) {
    spatialIndex = std::make_unique<OptimizedSpatialIndex>();
  } else {
    spatialIndex = std::make_unique<DefaultSpatialIndex>();
  }
}

void Environment::addOrganism(const std::shared_ptr<Organism> &organism, int x,
                              int y) {
  auto container =
      std::make_shared<SpatialObjectContainer>(organism, x, y, "Organism");
  spatialIndex->insert(container, x, y);
}

void Environment::addFood(int x, int y) {
  auto food = std::make_shared<Food>(x, y); // 假设Food构造函数接受位置参数
  auto container = std::make_shared<SpatialObjectContainer>(food, x, y, "Food");
  spatialIndex->insert(container, x, y);
}
