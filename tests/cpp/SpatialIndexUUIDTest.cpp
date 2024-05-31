#include <boost/uuid/uuid_hash.hpp>
#include <memory>
#include <test/SpatialIndexUUIDTest.hpp>

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
                            QueryReturnsCorrectResults, UpdateObjectCorrectly,
                            RemoveObjectCorrectly);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexUUIDTest, IndexTypes);
