#include "stdafx.h"
#include "CreateHeightMapAction.h"
#include "ObjWriter.h"
#include <fstream>
#include "MathUtils.h"

void CreateHeightMapAction::SaveObj()
{
    std::string fileName = file_ + ".obj";
    std::fstream f(fileName, std::fstream::out);
    ObjWriter writer(f);
    writer.Comment(file_);
    writer.Object("heightmap");

    if (vertices_.size() < 3)
        return;

    writer.Comment(std::to_string(vertices_.size()) + " vertices");
    for (const auto& v : vertices_)
    {
        writer.Vertex(v.x, v.y, v.z);
    }
    for (const auto& n : normals_)
    {
        writer.Normal(n.x, n.y, n.z);
    }
    writer.Comment(std::to_string(inidices_.size()) + " indices");
    for (size_t i = 0; i < inidices_.size(); )
    {
        writer.BeginFace();
        // Indices in OBJ are 1-based
        writer << inidices_[i] + 1 << inidices_[i + 1] + 1 << inidices_[i + 2] + 1;
        writer.EndFace();

        i += 3;
    }

    f.close();

    std::cout << "Created " << fileName << std::endl;
}

void CreateHeightMapAction::CreateGeometry()
{
    vertices_.resize(width_ * height_);
    normals_.resize(width_ * height_);
    for (int x = 0; x < width_; x++)
    {
        for (int z = 0; z < height_; z++)
        {
            float fy = GetRawHeight(x, z) * heightFactor_;
            float fx = (float)x - (float)width_ / 2.0f;
            float fz = (float)z - (float)height_ / 2.0f;
            vertices_[x * width_ + z] = {
                fx,
                fy,
                fz
            };

            normals_.push_back(GetRawNormal(x, z));
        }
    }

    // Create index data
    for (int z = 0; z < height_ - 1; z++)
    {
        for (int x = 0; x < width_ - 1; x++)
        {
            /*
            Normal edge:
            +----+----+
            |\ 1 |\   |
            | \  | \  |
            |  \ |  \ |
            | 2 \|   \|
            +----+----+
            */
            {
                // First triangle
                int i1 = z * width_ + x;
                int i2 = (z * width_) + x + 1;
                int i3 = (z + 1) * width_ + (x + 1);
                // P1
                inidices_.push_back(i1);
                // P2
                inidices_.push_back(i2);
                // P3
                inidices_.push_back(i3);
            }

            {
                // Second triangle
                int i3 = (z + 1) * width_ + (x + 1);
                int i2 = (z + 1) * width_ + x;
                int i1 = z * width_ + x;
                // P3
                inidices_.push_back(i3);
                // P2
                inidices_.push_back(i2);
                // P1
                inidices_.push_back(i1);
            }
        }
    }
}

float CreateHeightMapAction::GetRawHeight(int x, int z) const
{
    if (!data_)
        return 0.0f;

    x = Clamp(x, 0, width_ - 1);
    z = Clamp(z, 0, height_ - 1);
    int offset = (z * width_ + x) * components_;
    int i = 0;
    float r = (float)data_[offset + i++] / 256.0f;
    if (components_ > 1)
        r += (float)data_[offset + i++] / 256.0f;
    if (components_ > 2)
        r += (float)data_[offset + i++] / 256.0f;
    return r / i;
}

aiVector3D CreateHeightMapAction::GetRawNormal(int x, int z) const
{
    float baseHeight = GetRawHeight(x, z);
    float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    float up = 0.5f * (spacingX_ + spacingZ_);

    aiVector3D result = Vector3Add(aiVector3D(0.0f, up, nSlope), aiVector3D(-neSlope, up, neSlope));
    result = Vector3Add(result, aiVector3D(-eSlope, up, 0.0f));
    result = Vector3Add(result, aiVector3D(-seSlope, up, -seSlope));
    result = Vector3Add(result, aiVector3D(0.0f, up, -sSlope));
    result = Vector3Add(result, aiVector3D(swSlope, up, -swSlope));
    result = Vector3Add(result, aiVector3D(wSlope, up, 0.0f));
    result = Vector3Add(result, aiVector3D(nwSlope, up, nwSlope));
    // Normalize
    float length = sqrt(result.x * result.x + result.y * result.y + result.z * result.z);
    if (length > 0.0f)
    {
        result.x /= length;
        result.y /= length;
        result.z /= length;
    }
//    result.Normalize
    return result;
}

void CreateHeightMapAction::Execute()
{
    data_ = stbi_load(file_.c_str(), &width_, &height_, &components_, 3);

    if (!data_)
    {
        std::cout << "Error loading file " << file_ << std::endl;
        return;
    }

    CreateGeometry();

    SaveObj();

}
