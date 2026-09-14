//
//  ces_systems_feeder.cpp
//  gbCore
//
//  Created by Sergey Sergeev on 8/13/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "ces_systems_feeder.h"
#include "ces_base_system.h"
#include "scene_graph.h"
#include "ces_system_modifiers_component.h"

namespace gb
{
    ces_systems_feeder::ces_systems_feeder()
    {
        
    }
    
    ces_systems_feeder::~ces_systems_feeder()
    {
        
    }
    
    ces_base_system_shared_ptr ces_systems_feeder::get_system(i32 guid) const
    {
        const auto& system = m_systems.find(guid);
        if(system != m_systems.end())
        {
            return system->second;
        }
        return nullptr;
    }
    
    void ces_systems_feeder::on_update(f32 dt)
    {
        if(m_root)
        {
            while(!m_changed_entities.empty())
            {
                const auto it = m_changed_entities.front();
                m_changed_entities.pop();
                const auto entity = it.second.lock();
                if(entity)
                {
                    for (const auto& system : m_ordered_systems)
                    {
                        for (auto& required_mask : system->m_references_to_required_entities)
                        {
                            if (required_mask.second.size() > 2048)
                            {
                                std::cout<<"too much references for entitites: "<<required_mask.second.size()<<std::endl;
                                const auto system_modifiers_component = m_root->get_component<ces_system_modifiers_component>();
                                if (system_modifiers_component)
                                {
                                    system_modifiers_component->cleanup();
                                    system->cleanup(m_root);
                                    system_modifiers_component->cleanup_done();
                                    break;
                                }
                                else
                                {
                                    assert(false);
                                }
                            }
                            if(it.first == e_entity_state_changed || it.first == e_entity_state_removed)
                            {
                                required_mask.second.erase(std::remove_if(required_mask.second.begin(), required_mask.second.end(), [&entity](const ces_entity_weak_ptr& weak_entity) {
                                    bool result = weak_entity.lock() == entity;
                                    result |= weak_entity.expired();
                                    return result;
                                }), required_mask.second.end());
                            }
                            if(it.first == e_entity_state_changed || it.first == e_entity_state_added)
                            {
                                if(entity->is_components_exist(required_mask.first))
                                {
                                    required_mask.second.push_back(entity);
                                }
                            }
                        }
                    }
                }
            }
            
            auto scene = std::static_pointer_cast<scene_graph>(m_root);
            
            for(const auto& system : m_ordered_systems)
            {
                system->set_current_camera_2d(scene->get_camera_2d());
                system->set_current_camera_3d(scene->get_camera_3d());
                if (system->can_be_feeded(m_root))
                {
                    system->on_feed_start(dt);
                }
            }
            
            for(const auto& system : m_ordered_systems)
            {
                if (system->can_be_feeded(m_root))
                {
                    system->on_feed(m_root, dt);
                }
            }
            
            for(const auto& system : m_ordered_systems)
            {
                if (system->can_be_feeded(m_root))
                {
                    system->on_feed_end(dt);
                }
            }
            
            for(const auto& system : m_ordered_systems)
            {
                system->cleanup(m_root);
            }
            
            for(const auto& system : m_ordered_systems)
            {
                system->cleanup_done(m_root);
            }
        }
    }
    
    void ces_systems_feeder::add_system(const ces_base_system_shared_ptr& system)
    {
        m_systems[system->instance_guid()] = system;
        m_ordered_systems.push_back(system);
		m_ordered_systems.sort([](const ces_base_system_shared_ptr &value_01, const ces_base_system_shared_ptr &value_02) { 
			return value_01->get_order() < value_02->get_order(); 
		});
    }
    
    void ces_systems_feeder::remove_system(i32 type)
    {
        const auto& system = m_systems.find(type);
        if(system != m_systems.end())
        {
            const auto system_to_remove = system->second;
            m_systems.erase(system);
            const auto& iterator = std::find(m_ordered_systems.begin(), m_ordered_systems.end(), system_to_remove);
            if(iterator != m_ordered_systems.end())
            {
                m_ordered_systems.erase(iterator);
            }
            else
            {
                assert(false);
            }
        }
    }
    
    void ces_systems_feeder::set_root(const ces_entity_shared_ptr &entity)
    {
        m_root = entity;
    }
    
    void ces_systems_feeder::cleanup()
    {
        m_systems.clear();
        m_ordered_systems.clear();
        ces_systems_feeder::set_root(nullptr);
    }
    
    void ces_systems_feeder::on_entity_added(const ces_entity_shared_ptr& entity)
    {
         m_changed_entities.push(std::make_pair(e_entity_state_added, entity));
    }
    
    void ces_systems_feeder::on_entity_removed(const ces_entity_shared_ptr& entity)
    {
         m_changed_entities.push(std::make_pair(e_entity_state_removed, entity));
    }
    
    void ces_systems_feeder::on_entity_component_added(const ces_entity_shared_ptr& entity)
    {
        m_changed_entities.push(std::make_pair(e_entity_state_changed, entity));
    }
    
    void ces_systems_feeder::on_entity_component_removed(const ces_entity_shared_ptr& entity)
    {
         m_changed_entities.push(std::make_pair(e_entity_state_changed, entity));
    }
}
