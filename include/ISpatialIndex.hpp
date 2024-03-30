#ifndef ISPATIALINDEX_HPP
#define ISPATIALINDEX_HPP

#include "Organism.hpp"
#include <vector>

#include "SpatialObjectContainer.hpp"

class ISpatialIndex {
public:
  virtual void insert(const std::shared_ptr<SpatialObjectContainer> &object,
                      int x, int y) = 0;
  virtual std::vector<std::shared_ptr<Organism>> query(int x, int y,
                                                       int range) = 0;
  virtual ~ISpatialIndex();
};

class DefaultSpatialIndex : public ISpatialIndex {
public:
  DefaultSpatialIndex();
  void insert(const std::shared_ptr<SpatialObjectContainer> &object, int x,
              int y) override;
  std::vector<std::shared_ptr<Organism>> query(int x, int y,
                                               int range) override;
  ~DefaultSpatialIndex() override;
};

class OptimizedSpatialIndex : public ISpatialIndex {
public:
  OptimizedSpatialIndex();
  void insert(const std::shared_ptr<SpatialObjectContainer> &object, int x,
              int y) override;
  std::vector<std::shared_ptr<Organism>> query(int x, int y,
                                               int range) override;
  ~OptimizedSpatialIndex() override;
};

#endif
