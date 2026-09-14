//
//  mesh_3d_serializer_fbx.h
//  gbCore
//
//  Created by serhii.s on 9/14/26.
//

#pragma once

#include "resource_serializer.h"

namespace gb
{
    class mesh_3d_serializer_fbx final : public resource_serializer
    {
    private:

    protected:

        std::string m_filename;

    public:

        mesh_3d_serializer_fbx(const std::string& filename,
                               const resource_shared_ptr& resource);
        ~mesh_3d_serializer_fbx();

        void serialize(const resource_transfering_data_shared_ptr& transfering_data);
    };
};

