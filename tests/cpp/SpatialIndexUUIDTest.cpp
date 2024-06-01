#include <boost/uuid/uuid_hash.hpp>
#include <memory>
#include <test/SpatialIndexUUIDTest.hpp>

#include "gtest/gtest.h"

template <typename T>
void SpatialIndexUUIDTest<T>::SetUp() {
    index = std::make_unique<T>();
}

template <>
void SpatialIndexUUIDTest<OptimizedSpatialIndex<uuids::uuid>>::SetUp() {
    index = std::make_unique<OptimizedSpatialIndex<uuids::uuid>>(1000);
}

TYPED_TEST_P(SpatialIndexUUIDTest, InsertsObjectCorrectly) {
    auto object = uuids::random_generator()();
    EXPECT_NO_THROW(this->index->insert(object, 10, 10));
}

TYPED_TEST_P(SpatialIndexUUIDTest, QueryReturnsCorrectResults) {
    auto object = uuids::random_generator()();
    this->index->insert(object, 100, 100);
    auto results = this->index->query(100, 100, 1);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexUUIDTest, QueryReturnsCorrectResultsForManyObjects) {
    std::vector<uuids::uuid> objects;
    for (int i = 0; i < 10; i++) {
        auto object = uuids::random_generator()();
        this->index->insert(object, 20 + i, 20 + i);
        objects.push_back(object);
    }
    auto results = this->index->query(20, 20, 15);
    ASSERT_EQ(10, results.size());
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(results[i], objects[i]);
    }
}

TYPED_TEST_P(SpatialIndexUUIDTest, QueryFromFarAwayReturnsNoResultsForManyObjects) {
    std::vector<uuids::uuid> objects;
    for (int i = 0; i < 1000; i++) {
        auto object = uuids::random_generator()();
        this->index->insert(object, i, i);
        objects.push_back(object);
    }
    auto results = this->index->query(30, 0, 1);
    ASSERT_TRUE(results.empty());
}

// [TODO] The Optimized Spatial Index is not working in this test case. because it won't retrieve
// all the object in spatial index TYPED_TEST_P(SpatialIndexUUIDTest,
// QueryFarAwayButWithinRangeReturnsResultsForManyObjects) {
//     std::vector<uuids::uuid> objects;
//     for (int i = 0; i < 100; i++) {
//         auto object = uuids::random_generator()();
//         this->index->insert(object, i, i);
//         objects.push_back(object);
//     }
//     auto results = this->index->query(50, 50, 200);
//     ASSERT_EQ(100, results.size());
//     for (int i = 0; i < 100; i++) {
//         EXPECT_EQ(results[i], objects[i]);
//     }
// }

TYPED_TEST_P(SpatialIndexUUIDTest, UpdateObjectCorrectly) {
    auto object = uuids::random_generator()();
    this->index->insert(object, 10, 10);
    this->index->update(object, 70, 70);
    auto results = this->index->query(70, 70, 10);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexUUIDTest, RemoveObjectCorrectly) {
    auto object = uuids::random_generator()();
    this->index->insert(object, 10, 10);
    this->index->remove(object);
    auto results = this->index->query(20, 20, 10);
    ASSERT_TRUE(results.empty());
}

REGISTER_TYPED_TEST_SUITE_P(SpatialIndexUUIDTest, InsertsObjectCorrectly,
                            QueryReturnsCorrectResults, QueryReturnsCorrectResultsForManyObjects,
                            QueryFromFarAwayReturnsNoResultsForManyObjects,
                            // QueryFarAwayButWithinRangeReturnsResultsForManyObjects,
                            UpdateObjectCorrectly, RemoveObjectCorrectly);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexUUIDTest, IndexTypes);
