#ifndef _SPATIAL_OBJECT_CONTAINER
#define _SPATIAL_OBJECT_CONTAINER

#include "ISpatialObject.hpp"

class SpatialObjectContainer : public ISpatialObject {
public:
  SpatialObjectContainer(std::shared_ptr<void> obj, int x, int y,
                         std::string objType);

  std::pair<int, int> getPosition() const override;
  std::string getType() const override;

  template <typename T> std::shared_ptr<T> getObject() const;

private:
  std::shared_ptr<void> object;
  std::pair<int, int> position;
  std::string type;
};

#endif 
