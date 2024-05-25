#include <test/SpatialIndexTest.hpp>

template <typename T>
void SpatialIndexTest<T>::SetUp() {
    index = std::make_unique<T>();
}

template <>
void SpatialIndexTest<OptimizedSpatialIndex>::SetUp() {
    index = std::make_unique<OptimizedSpatialIndex>(1000);
}

template <typename T>
std::shared_ptr<ISpatialObject> SpatialIndexTest<T>::makeDummyObject(float x,
                                                                     float y) {
    Dummy dummy;
    return std::make_shared<SpatialObjectWrapper<Dummy>>(dummy, x, y);
}

TYPED_TEST_P(SpatialIndexTest, InsertsObjectCorrectly) {
    auto object = this->makeDummyObject(10.0f, 10.0f);
    EXPECT_NO_THROW(this->index->insert(object));
}

TYPED_TEST_P(SpatialIndexTest, QueryReturnsCorrectResults) {
    auto object = this->makeDummyObject(100.0f, 100.0f);
    this->index->insert(object);

    auto results = this->index->query(100.0f, 100.0f, 10.0f);
    ASSERT_EQ(1, results.size());
    EXPECT_EQ(object->getId(), results[0]);
}

REGISTER_TYPED_TEST_SUITE_P(SpatialIndexTest, InsertsObjectCorrectly,
                            QueryReturnsCorrectResults);

INSTANTIATE_TYPED_TEST_SUITE_P(DefaultIndexTests, SpatialIndexTest,
                               DefaultSpatialIndex);
INSTANTIATE_TYPED_TEST_SUITE_P(OptimizedIndexTests, SpatialIndexTest,
                               OptimizedSpatialIndex);

TEST(OptimizedSpatialIndexTest, ConstructorWithSize) {
    EXPECT_NO_THROW(OptimizedSpatialIndex(1000));
}
