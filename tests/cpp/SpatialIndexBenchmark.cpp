#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <chrono>
#include <cstdio>
#include <index/DefaultSpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <random>
#include <vector>

#include "gtest/gtest.h"

using namespace boost;

struct BenchmarkResult {
    double insertMs;
    double queryMs;
    double updateMs;
    double removeMs;
};

static const float WORLD_SIZE = 4000.0f;

// Helper: generate N random objects with positions
struct ObjectEntry {
    uuids::uuid id;
    float x, y;
};

static std::vector<ObjectEntry> generateObjects(int n, std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, WORLD_SIZE - 1.0f);
    uuids::random_generator gen;
    std::vector<ObjectEntry> objects;
    objects.reserve(n);
    for (int i = 0; i < n; i++) {
        objects.push_back({gen(), dist(rng), dist(rng)});
    }
    return objects;
}

template <typename IndexFactory>
BenchmarkResult runBenchmark(IndexFactory factory, int objectCount, int queryCount, float queryRange,
                             std::mt19937& rng) {
    auto index = factory();
    auto objects = generateObjects(objectCount, rng);
    std::uniform_real_distribution<float> posDist(0.0f, WORLD_SIZE - 1.0f);
    std::uniform_real_distribution<float> moveDist(-5.0f, 5.0f);
    BenchmarkResult result{};

    // Benchmark insert
    auto t0 = std::chrono::high_resolution_clock::now();
    for (auto& obj : objects) {
        index->insert(obj.id, obj.x, obj.y);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    result.insertMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Benchmark query - simulate each organism querying its neighbors
    std::vector<float> queryXs(queryCount), queryYs(queryCount);
    for (int i = 0; i < queryCount; i++) {
        queryXs[i] = posDist(rng);
        queryYs[i] = posDist(rng);
    }

    t0 = std::chrono::high_resolution_clock::now();
    volatile size_t totalResults = 0;  // prevent optimization
    for (int i = 0; i < queryCount; i++) {
        auto results = index->query(queryXs[i], queryYs[i], queryRange);
        totalResults += results.size();
    }
    t1 = std::chrono::high_resolution_clock::now();
    result.queryMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Benchmark update - simulate all organisms moving slightly each frame
    t0 = std::chrono::high_resolution_clock::now();
    for (auto& obj : objects) {
        float newX = std::max(0.0f, std::min(WORLD_SIZE - 1.0f, obj.x + moveDist(rng)));
        float newY = std::max(0.0f, std::min(WORLD_SIZE - 1.0f, obj.y + moveDist(rng)));
        index->update(obj.id, newX, newY);
        obj.x = newX;
        obj.y = newY;
    }
    t1 = std::chrono::high_resolution_clock::now();
    result.updateMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Benchmark remove
    t0 = std::chrono::high_resolution_clock::now();
    for (auto& obj : objects) {
        index->remove(obj.id);
    }
    t1 = std::chrono::high_resolution_clock::now();
    result.removeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return result;
}

static void printComparison(const char* label, const BenchmarkResult& def,
                            const BenchmarkResult& opt) {
    printf("\n=== %s ===\n", label);
    printf("%-12s %12s %12s %12s\n", "Operation", "Default(ms)", "Optimized(ms)", "Speedup");
    printf("%-12s %12.2f %12.2f %12.2fx\n", "Insert", def.insertMs, opt.insertMs,
           def.insertMs / opt.insertMs);
    printf("%-12s %12.2f %12.2f %12.2fx\n", "Query", def.queryMs, opt.queryMs,
           def.queryMs / opt.queryMs);
    printf("%-12s %12.2f %12.2f %12.2fx\n", "Update", def.updateMs, opt.updateMs,
           def.updateMs / opt.updateMs);
    printf("%-12s %12.2f %12.2f %12.2fx\n", "Remove", def.removeMs, opt.removeMs,
           def.removeMs / opt.removeMs);
    double defTotal = def.insertMs + def.queryMs + def.updateMs + def.removeMs;
    double optTotal = opt.insertMs + opt.queryMs + opt.updateMs + opt.removeMs;
    printf("%-12s %12.2f %12.2f %12.2fx\n", "TOTAL", defTotal, optTotal, defTotal / optTotal);
}

// Small scale: 200 objects, typical organism awareness range
TEST(SpatialIndexBenchmark, Small_200objects) {
    std::mt19937 rng(42);
    auto defResult = runBenchmark(
        []() { return std::make_unique<DefaultSpatialIndex<uuids::uuid>>(); }, 200, 200, 50.0f,
        rng);

    rng.seed(42);
    auto optResult = runBenchmark(
        []() { return std::make_unique<OptimizedSpatialIndex<uuids::uuid>>(WORLD_SIZE); }, 200, 200,
        50.0f, rng);

    printComparison("200 objects, range=50, 200 queries", defResult, optResult);
}

// Medium scale: 1000 objects
TEST(SpatialIndexBenchmark, Medium_1000objects) {
    std::mt19937 rng(42);
    auto defResult = runBenchmark(
        []() { return std::make_unique<DefaultSpatialIndex<uuids::uuid>>(); }, 1000, 1000, 50.0f,
        rng);

    rng.seed(42);
    auto optResult = runBenchmark(
        []() { return std::make_unique<OptimizedSpatialIndex<uuids::uuid>>(WORLD_SIZE); }, 1000,
        1000, 50.0f, rng);

    printComparison("1000 objects, range=50, 1000 queries", defResult, optResult);
}

// Large scale: 5000 objects - where quadtree should clearly win
TEST(SpatialIndexBenchmark, Large_5000objects) {
    std::mt19937 rng(42);
    auto defResult = runBenchmark(
        []() { return std::make_unique<DefaultSpatialIndex<uuids::uuid>>(); }, 5000, 5000, 50.0f,
        rng);

    rng.seed(42);
    auto optResult = runBenchmark(
        []() { return std::make_unique<OptimizedSpatialIndex<uuids::uuid>>(WORLD_SIZE); }, 5000,
        5000, 50.0f, rng);

    printComparison("5000 objects, range=50, 5000 queries", defResult, optResult);
}

// Simulate a full frame: insert all, update all (small move), query all, like Environment does
TEST(SpatialIndexBenchmark, SimulateFrame_1000objects) {
    const int N = 1000;
    const int FRAMES = 10;
    std::mt19937 rng(42);
    uuids::random_generator gen;
    std::uniform_real_distribution<float> posDist(0.0f, WORLD_SIZE - 1.0f);
    std::uniform_real_distribution<float> moveDist(-5.0f, 5.0f);

    // Generate shared objects
    std::vector<ObjectEntry> objects;
    objects.reserve(N);
    for (int i = 0; i < N; i++) {
        objects.push_back({gen(), posDist(rng), posDist(rng)});
    }

    auto runFrames = [&](auto makeIndex) {
        auto index = makeIndex();
        for (auto& obj : objects) {
            index->insert(obj.id, obj.x, obj.y);
        }

        // Copy positions so each run starts the same
        auto objs = objects;
        std::mt19937 frameRng(123);

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int frame = 0; frame < FRAMES; frame++) {
            // Update all positions (like updatePositionsInSpatialIndex)
            for (auto& obj : objs) {
                float newX = std::max(0.0f, std::min(WORLD_SIZE - 1.0f, obj.x + moveDist(frameRng)));
                float newY = std::max(0.0f, std::min(WORLD_SIZE - 1.0f, obj.y + moveDist(frameRng)));
                index->update(obj.id, newX, newY);
                obj.x = newX;
                obj.y = newY;
            }
            // Query all (like handleInteractions)
            for (auto& obj : objs) {
                auto results = index->query(obj.x, obj.y, 50.0f);
                (void)results;
            }
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(t1 - t0).count();
    };

    double defMs = runFrames([]() { return std::make_unique<DefaultSpatialIndex<uuids::uuid>>(); });
    double optMs = runFrames(
        []() { return std::make_unique<OptimizedSpatialIndex<uuids::uuid>>(WORLD_SIZE); });

    printf("\n=== Simulate %d frames, %d objects ===\n", FRAMES, N);
    printf("Default:   %.2f ms\n", defMs);
    printf("Optimized: %.2f ms\n", optMs);
    printf("Speedup:   %.2fx\n", defMs / optMs);
}
