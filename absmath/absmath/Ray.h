/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "Vector3.h"

namespace Math {

class BoundingBox;
class Sphere;
class Matrix4;
class Plane;

/// Infinite straight line in three-dimensional space.
class Ray
{
public:
    constexpr Ray() noexcept {}
    constexpr Ray(const Ray& other) noexcept :
        origin_(other.origin_),
        direction_(other.direction_)
    {}
    /// Construct from origin and direction.
    /// Hint: To get a Ray from one point to another use (target - origin) as direction
    Ray(const Vector3& origin, const Vector3& direction)
    {
        Define(origin, direction);
    }

    /// Assign from another ray.
    Ray& operator =(const Ray& rhs)
    {
        origin_ = rhs.origin_;
        direction_ = rhs.direction_;
        return *this;
    }

    /// Check for equality with another ray.
    bool operator ==(const Ray& rhs) const { return origin_.Equals(rhs.origin_) && direction_.Equals(rhs.direction_); }

    /// Check for inequality with another ray.
    bool operator !=(const Ray& rhs) const { return !origin_.Equals(rhs.origin_) || !direction_.Equals(rhs.direction_); }

    /// Define from origin and direction. The direction will be normalized.
    void Define(const Vector3& origin, const Vector3& direction)
    {
        origin_ = origin;
        direction_ = direction.Normal();
    }

    /// Project a point on the ray.
    Vector3 Project(const Vector3& point) const
    {
        const Vector3 offset = point - origin_;
        return origin_ + offset.DotProduct(direction_) * direction_;
    }

    /// Return distance of a point from the ray.
    float Distance(const Vector3& point) const
    {
        const Vector3 projected = Project(point);
        return (point - projected).Length();
    }

    /// Return closest point to another ray.
    Vector3 ClosestPoint(const Ray& ray) const;
    /// Return hit distance to a bounding box, or infinity if no hit.
    float HitDistance(const BoundingBox& box) const;
    /// Return hit distance to a sphere, or infinity if no hit.
    float HitDistance(const Sphere& sphere) const;
    float HitDistance(const Plane& plane) const;

    /// Return transformed by a 3x4 matrix. This may result in a non-normalized direction.
    Ray Transformed(const Matrix4& transform) const;
    bool IsDefined() const { return !origin_.Equals(Vector3::Zero) && !direction_.Equals(Vector3::Zero); }

    /// Ray origin.
    Vector3 origin_;
    /// Ray direction.
    Vector3 direction_;
};

template<class _Stream>
inline _Stream& operator << (_Stream& os, Ray& value)
{
    return os << value.origin_ << " -> " << value.direction_;
}

}
