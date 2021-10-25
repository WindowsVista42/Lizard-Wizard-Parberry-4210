// If you're wondering why this file exists its because me (sean) and ethan were talking and
// accidentally reinvented ecs. It turns out we s couple solid use-cases for an ecs so here it is.
//
// The data for entities is not stored in a global "MetaTable", instead the idea is to define
// all of the Tables in Game.h
//
// The benefit being that I don't have to go through and figure out how the hell to
// wrap up multiple tables into a singleton.
//
// While there are certainly other ways of organizing data, the power of a half-baked ecs where
// we can add and remove functionality by adding and removing the actual data is quite nice.
//
// This would probably be not-the-worst-thing-in-the-world to make thread-safe, considering
// its pretty inherently lock-free.

#ifndef ECS_H
#define ECS_H

#include "Defines.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>
#include <ctime>
#include <random>
#include <chrono>

class Entity {
    usize id;

    static usize& gen() {
        static usize gen= ~0;
        return gen;
    }

public:
    Entity() {
        gen() += 1;
        id = gen();
    }
};

template <typename T>
class Table {
    // Mapping of Handle --> index
    std::unordered_map<Entity, usize> mapping;

    // Data in a packed vector
    std::vector<T> data;

    // Store which entity we are a part of
    std::vector<Entity> entities;

public:
    // Add item to Table using Entity
    void Add(Entity e, T t) {
        data.push_back(t);
        mapping.insert(std::make_pair(e, data.size() - 1));
        entities.push_back(e);
    }

    // Remove item from Table
    void Remove(Entity e) {
        // Get the index of the item to be removed
        //if (mapping.count(e) != 1) { std::cout << typeid(T).name() << "Item not found!\n"; exit(1); }
        usize index = mapping[e];

        // Write last item to index
        data[index] = data[data.size() - 1];
        entities[index] = entities[entities.size() - 1];

        // Resize data
        data.resize(data.size() - 1);
        entities.resize(entities.size() - 1);

        // Remove old item
        mapping.erase(e);

        // Remap last item
        mapping[entities[index]] = index;
    }

    void Clear() {
        mapping.clear();
        data.clear();
        entities.clear();
    }

    // Get an item in the Table
    T* Get(Entity e) {
        //if (mapping.count(e) != 1) { std::cout << typeid(*this).name() << " Item not found!\n"; exit(1); }
        return &data[mapping[e]];
    }

    usize Size() {
        return data.size();
    }

    T* Data() {
        return data.data();
    }

    Entity* Entities() {
        return entities.data();
    }
};

class Group {
    std::unordered_map<Entity, usize> mapping;
    std::vector<Entity> entities;

public:
    void Add(Entity e) {
        entities.push_back(e);
        mapping.insert(std::make_pair(e, entities.size() - 1));
    }

    void Remove(Entity e) {
        usize index = mapping[e];

        // Write last item to index
        entities[index] = entities[entities.size() - 1];

        // Resize data
        entities.resize(entities.size() - 1);

        // Remove old item
        mapping.erase(e);

        // Remap last item
        mapping[entities[index]] = index;
    }

    void Clear() {
        entities.clear();
        mapping.clear();
    }

    usize Size() {
        return entities.size();
    }

    Entity* Entities() {
        return entities.data();
    }
};

#endif
