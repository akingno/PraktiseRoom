//
// Created by jacob on 2025/9/12.
//

#ifndef AK__RANDOM_H_
#define AK__RANDOM_H_

#include <random>
#include <atomic>
#include <chrono>

namespace AkRandom {

namespace detail {
    constexpr uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    // 全局原子计数器，确保每个线程拿到不同的种子
    inline std::atomic<uint64_t> g_thread_counter{0};

    // 确定性模式控制
    inline bool g_is_deterministic = false;
    inline uint64_t g_user_seed = 0;

    // 计算当前线程种子的帮助函数
    inline uint64_t get_thread_seed() {
        uint64_t thread_id = g_thread_counter.fetch_add(1);

        if (g_is_deterministic) {
            return splitmix64(g_user_seed + thread_id);
        } else {
            std::random_device rd;
            auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            return splitmix64(rd() ^ now ^ thread_id);
        }
    }
    // 线程局部状态包装器
    struct ThreadRNG {
        std::mt19937_64 engine;

        ThreadRNG() {
            engine.seed(get_thread_seed());
        }
    };

} // namespace detail

// --- API ---
inline std::mt19937_64& get_engine() {
  static thread_local detail::ThreadRNG t_rng_wrapper;
  return t_rng_wrapper.engine;
}

/**
 * @brief 初始化全局种子。
 * 调用后，所有后续创建的线程（以及主线程）将产生确定性的随机序列。
 */
inline void init(uint64_t seed) {
    detail::g_user_seed = seed;
    detail::g_is_deterministic = true;
    detail::g_thread_counter = 0;
    get_engine().seed(detail::splitmix64(seed));
}

/**
 * @brief 返回 [0, 1) 范围内的双精度浮点数
 */
inline double rand01() {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(get_engine());
}

/**
 * @brief 返回 [min, max) 范围内的双精度浮点数
 */
inline double randRange(double min, double max) {
  std::uniform_real_distribution<double> dist(min, max);
  return dist(get_engine());
}

/**
 * @brief 返回 [lo, hi] 闭区间内的整数
 */
inline int randint(int lo, int hi) {
  std::uniform_int_distribution<int> dist(lo, hi);
  return dist(get_engine());
}

/**
 * @brief 伯努利，以概率 p 返回 true
 */
inline bool bernoulli(double p) {
  if (p>=1)return true;
  if (p<=0)return false;

  std::bernoulli_distribution dist(p);
  return dist(get_engine());
}

/**
 * @brief 返回符合正态分布 N(mean, stddev) 的浮点数
 * @param mean 均值
 * @param stddev 标准差
 */
inline double randNormal(double mean, double stddev) {
  std::normal_distribution<double> dist(mean, stddev);
  return dist(get_engine());
}

} // namespace AkRandom

#endif
