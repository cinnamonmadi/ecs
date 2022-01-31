#pragma once

#ifndef NDEBUG
#   define ecs_assert(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

#include <cstdint>
#include <array>
#include <queue>
#include <unordered_map>
#include <set>
#include <iostream>

namespace ecs {
    const std::uint32_t MAX_ENTITIES = 4096;
    const std::uint8_t MAX_COMPONENTS = 32;

    using Entity = std::uint32_t;
    using ComponentType = std::uint8_t;
    using Signature = std::bitset<MAX_COMPONENTS>;

    class IComponentArray {
        public:
            virtual ~IComponentArray() = default;
            virtual void handle_entity_removed(Entity entity) = 0;
    };

    template<typename T>
    class ComponentArray : public IComponentArray {
        public:
            void insert_component(Entity entity, T component) {
                ecs_assert(entity_to_index_map.find(entity) == entity_to_index_map.end(), "Cannot insert component. Entity already has component of this type.");

                size_t new_index = size;
                entity_to_index_map[entity] = new_index;
                index_to_entity_map[new_index] = entity;
                values[new_index] = component;
                size++;
            }

            void remove_component(Entity entity) {
                ecs_assert(entity_to_index_map.find(entity) != entity_to_index_map.end(), "Cannot remove component. Entity doesn't have a component of this type.");

                // Swap removed component with the last component in the array, to ensure data remains tightly packed in the array
                size_t index_of_removed_entity = entity_to_index_map[entity];
                size_t index_of_last_component = size - 1;
                values[index_of_removed_entity] = values[index_of_last_component];

                // Update the map to be consistent with the above swap
                Entity entity_of_last_component = index_to_entity_map[index_of_last_component];
                entity_to_index_map[entity_of_last_component] = index_of_removed_entity;
                index_to_entity_map[index_of_removed_entity] = entity_of_last_component;

                entity_to_index_map.erase(entity);
                index_to_entity_map.erase(index_of_last_component);

                size--;
            }

            T& get_component(Entity entity) {
                ecs_assert(entity_to_index_map.find(entity) != entity_to_index_map.end(), "Cannot get component data. Entity doesn't have a component of this type.");

                return values[entity_to_index_map[entity]];
            }

            void handle_entity_removed(Entity entity) override {
                bool entity_had_component_of_this_type = entity_to_index_map.find(entity) != entity_to_index_map.end();

                if(entity_had_component_of_this_type) {
                    remove_component(entity);
                }
            }
        private:
            std::array<T, MAX_ENTITIES> values;
            std::unordered_map<Entity, size_t> entity_to_index_map;
            std::unordered_map<size_t, Entity> index_to_entity_map;
            std::size_t size;
    };

    class ECS {
        public:
            ECS() {
                entity_array_count = 0;
                component_arrays_count = 0;

                for(Entity i = 0; i < MAX_ENTITIES; i++) {
                    entity_available_ids.push(i);
                }
            }

            Entity create_entity() {
                ecs_assert(entity_array_count < MAX_ENTITIES, "Entity array is full.");

                Entity new_entity_id = entity_available_ids.front();
                entity_available_ids.pop();
                entity_array_count++;

                return new_entity_id;
            }

            void remove_entity(Entity entity_to_remove) {
                ecs_assert(entity_to_remove < MAX_ENTITIES, "Cannot remove entity. Entity out of range.");

                entity_signatures[entity_to_remove].reset();
                entity_available_ids.push(entity_to_remove);

                // Notify each component array that an entity has been destroyed
                for(auto const& pair : component_arrays) {
                    auto const& component_array = pair.second;
                    component_array->handle_entity_removed(entity_to_remove);
                }

                entity_array_count--;
            }

            Signature get_entity_signature(Entity entity) {
                ecs_assert(entity < MAX_ENTITIES, "Cannot get entity signature. Entity out of range.");

                return entity_signatures[entity];
            }

            template<typename T>
            void register_component() {
                const char* type_name = typeid(T).name();

                ecs_assert(component_types.find(type_name) == component_types.end(), "Cannot register component. Component type " + std::string(type_name) + " already registered.");

                component_types.insert({type_name, component_arrays_count});
                component_arrays.insert({type_name, std::make_shared<ComponentArray<T>>()});

                component_arrays_count++;
            }

            template<typename T>
            ComponentType get_component_type() {
                const char* type_name = typeid(T).name();

                ecs_assert(component_types.find(type_name) != component_types.end(), "Cannot get component type. Component of type " + std::string(type_name) + " not registered.");

                return component_types[type_name];
            }

            template<typename T>
            void add_component(Entity entity, T component) {
                get_component_array<T>()->insert_component(entity, component);
                entity_signatures[entity].set(get_component_type<T>());
            }

            template<typename T>
            void remove_component(Entity entity) {
                get_component_array<T>()->remove_component(entity);
                entity_signatures[entity].reset(get_component_type<T>());
            }

            template<typename T>
            T& get_component(Entity entity) {
                return get_component_array<T>()->get_component(entity);
            }

            template<typename ...rest>
            std::vector<Entity> view() {
                Signature system_signature;
                get_system_signature<rest...>(system_signature);

                std::vector<Entity> entity_list;
                for(Entity i = 0; i < entity_array_count; i++) {
                    if((entity_signatures[i] & system_signature) == system_signature) {
                        entity_list.push_back(i);
                    }
                }

                return entity_list;
            }
        private:
            std::queue<Entity> entity_available_ids;
            std::array<Signature, MAX_ENTITIES> entity_signatures;
            Entity entity_array_count;

            std::unordered_map<const char*, ComponentType> component_types;
            std::unordered_map<const char*, std::shared_ptr<IComponentArray>> component_arrays;
            ComponentType component_arrays_count;

            template<typename T>
            std::shared_ptr<ComponentArray<T>> get_component_array() {
                const char* type_name = typeid(T).name();

                ecs_assert(component_types.find(type_name) != component_types.end(), "Cannot get component array. Component of type " + std::string(type_name) + " not registered.");

                return std::static_pointer_cast<ComponentArray<T>>(component_arrays[type_name]);
            }

            template<typename T>
            void get_system_signature(Signature& signature) {
                signature.set(get_component_type<T>());
            }

            template<typename T, typename U, typename... rest>
            void get_system_signature(Signature& signature) {
                get_system_signature<T>(signature);
                get_system_signature<U, rest...>(signature);
            }

    };
};
