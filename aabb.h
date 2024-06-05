#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

class aabb
{
public:
	interval interval_x, interval_y, interval_z;

	aabb() {}

	aabb(const interval &x, const interval &y, const interval &z) : interval_x(x), interval_y(y), interval_z(z)
	{
		pad_to_minimums();
	}

	// ray与aabb的两个交点可以获得三个轴的边界
	aabb(const point3 &a, const point3 &b)
	{
		interval_x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
		interval_y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
		interval_z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);

		pad_to_minimums();
	}

	aabb(const aabb &box1, const aabb &box2)
	{
		interval_x = interval(box1.interval_x, box2.interval_x);
		interval_y = interval(box1.interval_y, box2.interval_y);
		interval_z = interval(box1.interval_z, box2.interval_z);
	}

	// 可根据标号n返回对应的轴区间
	const interval axis_interval(int n) const
	{
		if (n == 0)
			return interval_x;
		else if (n == 1)
			return interval_y;
		else
			return interval_z;
	}

	// judge if the ray hit the aabb or not
	bool hit(const ray &r, interval ray_t) const
	{
		const point3 &orig = r.origin();
		const vec3 &dir = r.direction();

		for (int axis = 0; axis < 3; axis++)
		{
			const interval &ax = axis_interval(axis);
			const double adinv = 1.0 / dir[axis];

			auto t0 = (ax.min - orig[axis]) * adinv;
			auto t1 = (ax.max - orig[axis]) * adinv;

			if (t0 < t1)
			{
				if (t0 > ray_t.min)
					ray_t.min = t0;
				if (t1 < ray_t.max)
					ray_t.max = t1;
			}
			else
			{
				if (t1 > ray_t.min)
					ray_t.min = t1;
				if (t0 < ray_t.max)
					ray_t.max = t0;
			}

			if (ray_t.min >= ray_t.max)
				return false;
		}
		return true;
	}

	// return the index of the longest axis of the bounding box
	int longest_axis()
	{
		if (interval_x.size() > interval_y.size())
		{
			return interval_x.size() > interval_z.size() ? 0 : 2;
		}
		else
		{
			return interval_y.size() > interval_z.size() ? 1 : 2;
		}
	}

	static const aabb empty, universe;

private:
	void pad_to_minimums()
	{
		// adjust the AABB so that no side is narrower than some delta, pading if necessary
		double delta = 0.0001;
		if (interval_x.size() < delta)
			interval_x = interval_x.expand(delta);
		if (interval_y.size() < delta)
			interval_y = interval_y.expand(delta);
		if (interval_z.size() < delta)
			interval_z = interval_z.expand(delta);
	}
};

const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

#endif
