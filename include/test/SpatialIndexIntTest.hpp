#ifndef SPATIALINDEXTEST_HPP
#define SPATIALINDEXTEST_HPP

#include <gtest/gtest.h>

#include <index/ISpatialIndex.hpp>
#include <index/DefaultSpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <memory>

template <typename T>
class SpatialIndexIntTest : public ::testing::Test {
protected:
    std::unique_ptr<ISpatialIndex<int>> index;

    void SetUp() override;
};

using IndexTypes = ::testing::Types<DefaultSpatialIndex<int>, OptimizedSpatialIndex<int>>;

TYPED_TEST_SUITE_P(SpatialIndexIntTest);

#endif
