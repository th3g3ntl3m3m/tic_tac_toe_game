// LRUCache.h
#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <list>
#include <mutex>

template<typename K, typename V>
class LRUCache {
private:
    size_t max_size;
    std::list<K> keys;
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> cache;
    mutable std::mutex cache_mutex;

public:
    LRUCache(size_t size) : max_size(size) {}

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(key);

        if (it != cache.end()) {
            keys.erase(it->second.second);
            cache.erase(it);
        } else if (keys.size() >= max_size) {
            cache.erase(keys.back());
            keys.pop_back();
        }

        keys.push_front(key);
        cache[key] = {value, keys.begin()};
    }

    bool get(const K& key, V& value) const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(key);

        if (it == cache.end()) {
            return false;
        }

        keys.splice(keys.begin(), keys, it->second.second);
        value = it->second.first;
        return true;
    }
};

#endif // LRUCACHE_H
