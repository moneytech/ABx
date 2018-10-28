#pragma once

#include "Resource.h"

namespace Resources {

class TemplateResource : public Resource
{
protected:
    std::vector<std::string> headerScripts_;
    std::vector<std::string> footerScripts_;
    std::vector<std::string> styles_;
    virtual bool GetObjects(std::map<std::string, ginger::object>& objects);
    virtual std::string GetTemplate() = 0;
public:
    explicit TemplateResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
