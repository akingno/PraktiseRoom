//
// Created by jacob on 26-2-19.
//
#include "Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void Cfg::load(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cout << "[Config] " << filename << " not found, generating default config..." << std::endl;

    if (need_rules.empty()) {
      need_rules.push_back({"hunger", 2.0, 60.0, 0.0, 1.5});
      need_rules.push_back({"fatigue", 0.5, 60.0, 25.0, 1.2});
      need_rules.push_back({"boredom", 0.8, 40.0, 5.0, 1.0});
    }

    save(filename);
    return;
  }

  try {
    json j;
    file >> j;

    if (j.contains("core")) {
      core::tile_px = j["core"].value("tile_px", core::tile_px);
      core::tick_milli = j["core"].value("tick_milli", core::tick_milli);
      core::tick_milli_int = j["core"].value("tick_milli_int", core::tick_milli_int);
      core::ticks_per_sec = j["core"].value("ticks_per_sec", core::ticks_per_sec);
    }

    if (j.contains("room")) {
      room::view_w = j["room"].value("view_w", room::view_w);
      room::view_h = j["room"].value("view_h", room::view_h);
      room::door_x = j["room"].value("door_x", room::door_x);
      room::door_y = j["room"].value("door_y", room::door_y);
      room::food_x = j["room"].value("food_x", room::food_x);
      room::food_y = j["room"].value("food_y", room::food_y);
      room::bed_x = j["room"].value("bed_x", room::bed_x);
      room::bed_y = j["room"].value("bed_y", room::bed_y);
      room::computer_x = j["room"].value("computer_x", room::computer_x);
      room::computer_y = j["room"].value("computer_y", room::computer_y);
    }

    if (j.contains("speed")) {
      speed::sleep_recover = j["speed"].value("sleep_recover", speed::sleep_recover);
      speed::computer_recover = j["speed"].value("computer_recover", speed::computer_recover);
    }

    if (j.contains("score")) {
      score::base_wander = j["score"].value("base_wander", score::base_wander);
      score::base_stop = j["score"].value("base_stop", score::base_stop);
      score::base_use_computer = j["score"].value("base_use_computer", score::base_use_computer);
      score::base_talk = j["score"].value("base_talk", score::base_talk);
    }

    if (j.contains("time")) {
      time::min_stop = j["time"].value("min_stop", time::min_stop);
      time::max_stop = j["time"].value("max_stop", time::max_stop);
      time::min_use_computer = j["time"].value("min_use_computer", time::min_use_computer);
      time::max_use_computer = j["time"].value("max_use_computer", time::max_use_computer);
    }
    if (j.contains("needs")) {
      need_rules.clear();
      for (const auto &item : j["needs"]) {
        NeedRule rule;
        rule.name = item.value("name", "unknown");
        rule.growth_rate = item.value("growth_rate", 1.0);
        rule.enter_threshold = item.value("enter_threshold", 50.0);
        rule.exit_threshold = item.value("exit_threshold", 0.0);
        rule.weight = item.value("weight", 1.0);
        need_rules.push_back(rule);
      }
    }

    if (j.contains("prob")) {
      prob::change_action = j["prob"].value("change_action", prob::change_action);
      prob::change_talk = j["prob"].value("change_talk", prob::change_talk);
    }

    if (j.contains("item")) {
      item::food_calories = j["item"].value("food_calories", item::food_calories);
      item::play_computer_entertain = j["item"].value("play_computer_entertain", item::play_computer_entertain);
    }

    std::cout << "[Config] Loaded configuration from " << filename << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[Config] Failed to parse JSON: " << e.what() << std::endl;
  }
}

void Cfg::save(const std::string &filename) {
  json j;

  j["core"] = {
      {"tile_px", core::tile_px},
      {"tick_milli", core::tick_milli},
      {"tick_milli_int", core::tick_milli_int},
      {"ticks_per_sec", core::ticks_per_sec}};

  j["room"] = {
      {"view_w", room::view_w},
      {"view_h", room::view_h},
      {"door_x", room::door_x},
      {"door_y", room::door_y},
      {"food_x", room::food_x},
      {"food_y", room::food_y},
      {"bed_x", room::bed_x},
      {"bed_y", room::bed_y},
      {"computer_x", room::computer_x},
      {"computer_y", room::computer_y}};

  j["speed"] = {
      {"sleep_recover", speed::sleep_recover},
      {"computer_recover", speed::computer_recover}};

  j["needs"] = json::array();
  for (const auto &rule : need_rules) {
    j["needs"].push_back({{"name", rule.name},
                          {"growth_rate", rule.growth_rate},
                          {"enter_threshold", rule.enter_threshold},
                          {"exit_threshold", rule.exit_threshold},
                          {"weight", rule.weight}});
  }

  j["score"] = {
      {"base_wander", score::base_wander},
      {"base_stop", score::base_stop},
      {"base_use_computer", score::base_use_computer},
      {"base_talk", score::base_talk}};

  j["time"] = {
      {"min_stop", time::min_stop},
      {"max_stop", time::max_stop},
      {"min_use_computer", time::min_use_computer},
      {"max_use_computer", time::max_use_computer}};

  j["prob"] = {
      {"change_action", prob::change_action},
      {"change_talk", prob::change_talk}};

  j["item"] = {
      {"food_calories", item::food_calories},
      {"play_computer_entertain", item::play_computer_entertain}};

  std::ofstream file(filename);
  if (file.is_open()) {
    file << j.dump(4);
    std::cout << "[Config] Saved configuration to " << filename << std::endl;
  } else {
    std::cerr << "[Config] Error saving configuration to " << filename << std::endl;
  }
}