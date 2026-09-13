//
//  resource_serializer.cpp
//  gbCore
//
//  Created by Sergey Sergeev on 8/12/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "resource_serializer.h"
#include "resource.h"
#include "common.h"

namespace gb
{
    resource_serializer::resource_serializer(const std::string& guid,
                                             const resource_shared_ptr& resource) :
    m_guid(guid),
    m_resource(resource),
    m_status(e_serializer_status_undefined)
    {
        
    }
    
    std::shared_ptr<std::istream> resource_serializer::open_stream(const std::string &filename, e_serializer_status* status)
    {
        std::shared_ptr<std::ifstream> filestream = std::make_shared<std::ifstream>();
        filestream->open(filename.c_str(), std::ifstream::binary);
        if(!filestream->is_open())
        {
            filestream->open(bundlepath().append(filename).c_str(), std::ifstream::binary);
            if (!filestream->is_open())
            {
                *status = e_serializer_status_failure;
				std::cerr << "missing resource: " << filename << std::endl;
                assert(false);
            }
        }
        return filestream;
    }
    
    void resource_serializer::close_stream(const std::shared_ptr<std::istream> &stream)
    {
        std::static_pointer_cast<std::ifstream>(stream)->close();
    }
    
    void resource_serializer::on_transfering_data_serialized(const resource_transfering_data_shared_ptr &transfering_data)
    {
        m_resource->on_transfering_data_serialized(transfering_data);
    }
    
    std::string resource_serializer::get_guid() const
    {
        return m_guid;
    }
    
    e_serializer_status resource_serializer::get_status() const
    {
        return m_status;
    }
}
