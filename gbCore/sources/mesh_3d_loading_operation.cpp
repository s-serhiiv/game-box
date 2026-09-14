//
//  mesh_3d_loading_operation.cpp
//  gbCore
//
//  Created by serhii serhiiv on 5/15/17.
//  Copyright © 2017 sergey.sergeev. All rights reserved.
//

#include "mesh_3d_loading_operation.h"
#include "mesh_3d.h"
#include "mesh_3d_serializer_gbmesh3d.h"
#include "mesh_3d_serializer_fbx.h"
#include "mesh_3d_commiter_gbmesh3d.h"

namespace gb
{
    mesh_3d_loading_operation::mesh_3d_loading_operation(const std::string& filename, const resource_shared_ptr& resource) :
    gb::resource_loading_operation(filename, resource),
    m_filename(filename)
    {
        m_transfering_data = nullptr;
    }
    
    mesh_3d_loading_operation::~mesh_3d_loading_operation()
    {
        
    }
    
    void mesh_3d_loading_operation::serialize()
    {
        assert(m_resource);
        m_status = e_resource_loading_operation_status_in_progress;
        
        if(m_filename.find(".GB3DMESH") != std::string::npos || m_filename.find(".GB3D_MESH") != std::string::npos)
        {
            m_serializer = std::make_shared<mesh_3d_serializer_gbmesh3d>(m_filename, m_resource);
        }
        else if(m_filename.find(".fbx") != std::string::npos || m_filename.find(".FBX") != std::string::npos)
        {
            m_serializer = std::make_shared<mesh_3d_serializer_fbx>(m_filename, m_resource);
        }
        else
        {
            assert(false);
        }
        
        m_serializer->serialize(m_transfering_data);
        m_status = m_serializer->get_status() == e_serializer_status_success ? e_resource_loading_operation_status_waiting : e_resource_loading_operation_status_failure;
    }
    
    void mesh_3d_loading_operation::commit()
    {
        assert(m_resource);
        assert(m_resource->is_loaded());
        
        if(m_filename.find(".GB3DMESH") != std::string::npos || m_filename.find(".GB3D_MESH") != std::string::npos)
        {
            m_commiter = std::make_shared<mesh_3d_commiter_gbmesh3d>(m_serializer->get_guid(), m_resource);
        }
        else if(m_filename.find(".fbx") != std::string::npos || m_filename.find(".FBX") != std::string::npos)
        {
            m_commiter = std::make_shared<mesh_3d_commiter_gbmesh3d>(m_serializer->get_guid(), m_resource);
        }
        else
        {
            assert(false);
        }
        
        m_commiter->commit(m_transfering_data);
        m_status = m_commiter->get_status() == e_commiter_status_success ? e_resource_loading_operation_status_success : e_resource_loading_operation_status_failure;
    }
}
