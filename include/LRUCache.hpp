#pragma once

#include <list>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <string_view>
#include <vector>

#include "ICache.hpp"

namespace SKW_WPX {
    
    namespace Cache {

        class LRUCache : public ICache {
        public:
            explicit LRUCache(size_t maxEntries = 1024) : maxCacheSize(maxEntries) {}

            LRUCache(const LRUCache&) = delete;
            LRUCache(LRUCache&&) = delete;
            LRUCache& operator=(const LRUCache&) = delete;
            LRUCache& operator=(LRUCache&&) = delete;

            void put(const std::string& key, int value) override {
                std::scoped_lock lock(mtx);
                auto it = map.find(key);

                if (it != map.end()) {
                    keys.erase(it->second.second);
                } else if (keys.size() >= maxCacheSize) {
                    auto last = keys.back();
                    keys.pop_back();
                    map.erase(last);
                }

                keys.emplace_front(key);
                map[keys.front()] = {value, keys.begin()};
            }

            std::optional<int> get(const std::string& key) override {
                std::scoped_lock lock(mtx);
                auto it = map.find(key);
                if (it == map.end()) return std::nullopt;

                keys.erase(it->second.second);
                keys.push_front(key);
                it->second.second = keys.begin();

                return it->second.first;
            }

            bool contains(const std::string& key) const {
                std::scoped_lock lock(mtx);
                return map.find(key) != map.end();
            }

            bool erase(const std::string& key) {
                std::scoped_lock lock(mtx);
                auto it = map.find(key);
                if (it == map.end()) return false;

                keys.erase(it->second.second);
                map.erase(it);
                return true;
            }

            std::vector<std::string> get_all_keys() const {
                std::scoped_lock lock(mtx);
                return {keys.begin(), keys.end()};
            }

            std::optional<std::string> get_lru_key() const {
                std::scoped_lock lock(mtx);
                if (keys.empty()) return std::nullopt;
                return keys.back();
            }

            size_t size() const {
                std::scoped_lock lock(mtx);
                return keys.size();
            }

            void resize(size_t newMaxEntries) {
                std::scoped_lock lock(mtx);
                maxCacheSize = newMaxEntries;
                while (keys.size() > maxCacheSize) {
                    auto last = keys.back();
                    keys.pop_back();
                    map.erase(last);
                }
            }

            void clear() {
                std::scoped_lock lock(mtx);
                map.clear();
                keys.clear();
            }

        private:
            mutable std::mutex mtx;
            std::list<std::string> keys;
            std::unordered_map<std::string_view, std::pair<int, std::list<std::string>::iterator>> map;
            size_t maxCacheSize;
        };
    }
}