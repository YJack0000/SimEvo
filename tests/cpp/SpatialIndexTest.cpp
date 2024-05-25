#include <boost/uuid/uuid_generators.hpp>
#include <test/SpatialIndexTest.hpp>

template <typename T>
void SpatialIndexTest<T>::SetUp() {
    index = std::make_unique<T>();
}

// template <>
// void SpatialIndexTest<OptimizedSpatialIndex<boost::uuids::uuid>>::SetUp() {
//     index =
//     std::make_unique<OptimizedSpatialIndex<boost::uuids::uuid>>(1000);
// }

TYPED_TEST_P(SpatialIndexTest, InsertsObjectCorrectly) {
    auto object = boost::uuids::random_generator()();
    EXPECT_NO_THROW(this->index->insert(object, 10, 10));
}

TYPED_TEST_P(SpatialIndexTest, QueryReturnsCorrectResults) {
    auto object = boost::uuids::random_generator()();
    this->index->insert(object, 100, 100);
    auto results = this->index->query(100, 100, 10);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexTest, UpdateObjectCorrectly) {
    auto object = boost::uuids::random_generator()();
    this->index->insert(object, 10, 10);
    this->index->update(object, 70, 70);
    auto results = this->index->query(70, 70, 10);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexTest, RemoveObjectCorrectly) {
    auto object = boost::uuids::random_generator()();
    this->index->insert(object, 10, 10);
    this->index->remove(object);
    auto results = this->index->query(20, 20, 10);
    ASSERT_TRUE(results.empty());
}

REGISTER_TYPED_TEST_SUITE_P(SpatialIndexTest, InsertsObjectCorrectly,
                            QueryReturnsCorrectResults, UpdateObjectCorrectly,
                            RemoveObjectCorrectly);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexTest,
                               DefaultSpatialIndex<boost::uuids::uuid>);
