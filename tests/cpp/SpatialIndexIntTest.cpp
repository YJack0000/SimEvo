#include <test/SpatialIndexIntTest.hpp>

template <typename T>
void SpatialIndexIntTest<T>::SetUp() {
    index = std::make_unique<T>();
}

template <>
void SpatialIndexIntTest<OptimizedSpatialIndex<int>>::SetUp() {
    index = std::make_unique<OptimizedSpatialIndex<int>>(1000);
}

TYPED_TEST_P(SpatialIndexIntTest, InsertsObjectCorrectly) {
    int object = 1;
    EXPECT_NO_THROW(this->index->insert(object, 10, 10));
}

TYPED_TEST_P(SpatialIndexIntTest, QueryReturnsCorrectResults) {
    auto object = 1;
    this->index->insert(object, 100, 100);
    auto results = this->index->query(100, 100, 1);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexIntTest, UpdateObjectCorrectly) {
    auto object = 1;
    this->index->insert(object, 10, 10);
    this->index->update(object, 70, 70);
    auto results = this->index->query(70, 70, 10);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0], object);
}

TYPED_TEST_P(SpatialIndexIntTest, RemoveObjectCorrectly) {
    auto object = 1;
    this->index->insert(object, 10, 10);
    this->index->remove(object);
    auto results = this->index->query(20, 20, 10);
    ASSERT_TRUE(results.empty());
}

REGISTER_TYPED_TEST_SUITE_P(SpatialIndexIntTest, InsertsObjectCorrectly, QueryReturnsCorrectResults,
                            UpdateObjectCorrectly, RemoveObjectCorrectly);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexIntTest, IndexTypes);
