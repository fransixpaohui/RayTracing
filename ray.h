#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray
{
public:
	ray() {}
	ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction), tm(0) {}
	ray(const point3& origin, const vec3& direction, double time) : orig(origin), dir(direction), tm(time) {}

	// 获取光线的两个参数，并且可作为左值
	const point3& origin() const
	{
		return this->orig;
	}
	const vec3& direction() const
	{
		return this->dir;
	}

	double time() const
	{
		return tm;
	}

	// 利用at获取ray在t时的的点位
	point3
		at(double t) const
	{
		return orig + dir * t;
	}

private:
	point3 orig;
	vec3 dir;
	double tm;
};

#endif