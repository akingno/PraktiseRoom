//
// Created by jacob on 26-3-10.
//

#ifndef WORLDMANAGER_H
#define WORLDMANAGER_H

#include "Room.h"
#include "ItemLayer.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"

using json = nlohmann::json;

struct Level {
    int level_id;
    std::unique_ptr<Room> room;
    std::unique_ptr<ItemLayer> items;
};

class WorldManager {
public:
    int current_active_level = 0; // 当前聚焦的层级

    static WorldManager& inst() {
        static WorldManager instance;
        return instance;
    }

    Level* getActiveLevel() {
        return getLevel(current_active_level);
    }

    Level* getLevel(int level_id) {
        auto it = levels_.find(level_id);
        return it != levels_.end() ? it->second.get() : nullptr;
    }

    Level* createLevel(int level_id, int w, int h) {
        if (levels_.find(level_id) != levels_.end()) {
            return levels_[level_id].get();
        }
        auto lvl = std::make_unique<Level>();
        lvl->level_id = level_id;
        lvl->room = std::make_unique<Room>(w, h);
        lvl->items = std::make_unique<ItemLayer>(w, h, level_id);
        lvl->items->initDefaultLayout();

        Level* ptr = lvl.get();
        levels_[level_id] = std::move(lvl);
        spdlog::info("WorldManager: Created Level {}", level_id);
        return ptr;
    }

    void saveAllWorlds(const std::string& meta_file = "world_meta.json") {
        json jArray = json::array();
        for (const auto& [id, lvl] : levels_) {
            jArray.push_back(id);
            lvl->room->saveToFile("room_map_" + std::to_string(id) + ".json");
            lvl->items->saveToFile("world_items_" + std::to_string(id) + ".json");
        }
        std::ofstream out(meta_file);
        if (out.is_open()) {
            out << jArray.dump(4);
            spdlog::info("WorldManager: Saved multi-level meta index to {}", meta_file);
        }
    }

    void loadAllWorlds(const std::string& meta_file = "world_meta.json") {
        clear();
        std::ifstream file(meta_file);
        if (!file.is_open()) {
            spdlog::info("WorldManager: Meta file not found, initializing Level 0 only.");
            createLevel(0, Cfg::room::view_w, Cfg::room::view_h);
            return;
        }
        try {
            json jArray;
            file >> jArray;
            for (int id : jArray) {
                auto* lvl = createLevel(id, Cfg::room::view_w, Cfg::room::view_h);
                lvl->room->loadFromFile("room_map_" + std::to_string(id) + ".json");
                lvl->items->loadFromFile("world_items_" + std::to_string(id) + ".json");
            }
            spdlog::info("WorldManager: Loaded {} levels.", jArray.size());
        } catch (const std::exception& e) {
            spdlog::error("WorldManager: Failed to load meta JSON: {}", e.what());
            createLevel(0, Cfg::room::view_w, Cfg::room::view_h);
        }
    }

    const std::unordered_map<int, std::unique_ptr<Level>>& getAllLevels() const {
        return levels_;
    }

    void clear() { levels_.clear(); }

private:
    std::unordered_map<int, std::unique_ptr<Level>> levels_;
};

#endif //WORLDMANAGER_H
