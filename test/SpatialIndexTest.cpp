#include <memory>
#include <test/SpatialIndexTest.hpp>

TYPED_TEST_SUITE_P(SpatialIndexTest);

TYPED_TEST_P(SpatialIndexTest, InsertsObjectCorrectly) {
    auto object = this->makeDummyObject(10, 10);
    EXPECT_NO_THROW(this->index->insert(object));
}

TYPED_TEST_P(SpatialIndexTest, QueryReturnsCorrectResults) {
    auto object = this->makeDummyObject(100, 100);
    this->index->insert(object);

    auto results = this->index->query(100, 100, 10);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(results[0]->getPosition(), std::make_pair(100, 100));
}

REGISTER_TYPED_TEST_SUITE_P(SpatialIndexTest, InsertsObjectCorrectly,
                            QueryReturnsCorrectResults);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexTest,
                               DefaultSpatialIndex);
// INSTANTIATE_TYPED_TEST_SUITE_P(OptimizedIndexTests, SpatialIndexTest,
                               // OptimizedSpatialIndex);
