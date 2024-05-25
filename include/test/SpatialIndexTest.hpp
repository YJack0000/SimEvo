#ifndef SPATIALINDEXTEST_HPP
#define SPATIALINDEXTEST_HPP

#include <gtest/gtest.h>

#include <index/ISpatialIndex.hpp>
#include <index/ISpatialObject.hpp>
#include <index/SpatialIndex.hpp>
#include <index/SpatialObjectWrapper.hpp>
#include <memory>
#include <test/Dummy.hpp>

template <typename T>
class SpatialIndexTest : public ::testing::Test {
protected:
    std::unique_ptr<ISpatialIndex> index;

    void SetUp() override;
    std::shared_ptr<ISpatialObject> makeDummyObject(float x, float y);
};

using IndexTypes = ::testing::Types<DefaultSpatialIndex, OptimizedSpatialIndex>;
TYPED_TEST_SUITE(SpatialIndexTest, IndexTypes);

#endif
