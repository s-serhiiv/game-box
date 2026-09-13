//
//  cubemap_texture_serializer_png.cpp
//  gbCore
//
//  Created by serhii.s on 5/14/19.
//  Copyright © 2019 sergey.sergeev. All rights reserved.
//

#include "cubemap_texture_serializer_png.h"
#include "texture_serializer_png.h"
#include "cubemap_texture.h"

namespace gb
{
    cubemap_texture_serializer_png::cubemap_texture_serializer_png(const std::string& filename,
                                                                   const resource_shared_ptr& resource) :
    gb::resource_serializer(filename, resource),
    m_filename(filename)
    {
        
    }
    
    void cubemap_texture_serializer_png::serialize(const resource_transfering_data_shared_ptr& transfering_data)
    {
        assert(m_resource != nullptr);
        m_status = e_serializer_status_in_progress;
        
        const auto texture_transfering_data = std::static_pointer_cast<gb::cubemap_texture_transfering_data>(transfering_data);
        texture_transfering_data->m_bpp = 8;
        texture_transfering_data->m_mips = 0;
        texture_transfering_data->m_compressed = false;
        
        std::array<std::string, 6> slice_suffix;
        slice_suffix[0] = "_px.png";
        slice_suffix[1] = "_nx.png";
        slice_suffix[2] = "_py.png";
        slice_suffix[3] = "_ny.png";
        slice_suffix[4] = "_pz.png";
        slice_suffix[5] = "_nz.png";
        
        for (ui32 slice = 0; slice < 6; ++slice)
        {
            std::string filename = m_filename;
            filename.append(slice_suffix.at(slice));
            std::shared_ptr<std::istream> filestream = resource_serializer::open_stream(filename, &m_status);
            const auto result = texture_serializer_png::read_png(filestream);
            
            texture_transfering_data->m_data[slice] = std::get<0>(result);
            texture_transfering_data->m_size = std::get<1>(result);
            texture_transfering_data->m_format = std::get<3>(result);
            assert(std::get<1>(result) == std::get<2>(result));
            
            resource_serializer::close_stream(filestream);
        }
       
        m_status = e_serializer_status_success;
        resource_serializer::on_transfering_data_serialized(texture_transfering_data);
    }
}
