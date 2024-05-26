#ifndef SPATIALINDEXTEST_HPP
#define SPATIALINDEXTEST_HPP

#include <gtest/gtest.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <index/ISpatialIndex.hpp>
#include <index/SpatialIndex.hpp>
#include <memory>

template <typename T>
class SpatialIndexTest : public ::testing::Test {
protected:
    std::unique_ptr<ISpatialIndex<int>> index;

    void SetUp() override;
};

using IndexTypes = ::testing::Types<DefaultSpatialIndex<int>,
                                    OptimizedSpatialIndex<int>>;

TYPED_TEST_SUITE_P(SpatialIndexTest);

#endif
