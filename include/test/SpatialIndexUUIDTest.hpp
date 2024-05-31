
#ifndef SPATIALINDEXTEST_HPP
#define SPATIALINDEXTEST_HPP

#include <gtest/gtest.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <index/DefaultSpatialIndex.hpp>
#include <index/ISpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <memory>

using namespace boost;

template <typename T>
class SpatialIndexUUIDTest : public ::testing::Test {
protected:
    std::unique_ptr<ISpatialIndex<uuids::uuid>> index;
    void SetUp() override;
};

using IndexTypes =
    ::testing::Types<DefaultSpatialIndex<uuids::uuid>, OptimizedSpatialIndex<uuids::uuid>>;
TYPED_TEST_SUITE_P(SpatialIndexUUIDTest);

#endif
