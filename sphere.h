#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "onb.h"

class sphere : public hittable
{
private:
	point3 center1;
	double radius;
	std::shared_ptr<material> mat;
	bool is_moving;
	vec3 center_vec;
	aabb bbox;

	// Linearly interpolate from center1 to center2 according to time,where t = 0 yields center1,t = 1 yields center2
	point3 sphere_center(double time) const
	{
		return center1 + time * center_vec;
	}

	static vec3 random_to_sphere(double radius, double distance_squared) {
		auto r1 = random_double();
		auto r2 = random_double();
		auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

		auto phi = 2 * pi * r1;
		auto x = cos(phi) * sqrt(1 - z * z);
		auto y = sin(phi) * sqrt(1 - z * z);

		return vec3(x, y, z);
	}

public:
	// Stationary Sphere
	sphere(const point3 &center, const double &radius, std::shared_ptr<material> mat) : center1(center), radius(fmax(0, radius)), mat(mat), is_moving(false)
	{
		auto rvec = vec3(radius, radius, radius);
		bbox = aabb(center1 - rvec, center1 + rvec);
	}

	// Moving Sphere
	sphere(const point3 &center1, const point3 &center2, double radius, std::shared_ptr<material> mat) : center1(center1), radius(fmax(0, radius)), mat(mat), is_moving(true)
	{
		center_vec = center2 - center1;

		auto rvec = vec3(radius, radius, radius);
		aabb box1(center1 - rvec, center1 + rvec);
		aabb box2(center2 - rvec, center2 + rvec);
		bbox = aabb(box1, box2);
	}

	// 判断光线与物体是否相交
	bool hit(const ray &r, interval ray_t, hit_record &rec) const override
	{
		// determin the center
		point3 center = is_moving ? sphere_center(r.time()) : center1;
		vec3 oc = center - r.origin();
		auto a = r.direction().length_squared();
		auto h = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius * radius;

		auto discriminant = h * h - a * c;
		if (discriminant < 0)
			return false;

		auto sqrtd = sqrt(discriminant);

		// Find the nearest root that lies in the acceptable range.
		auto root = (h - sqrtd) / a;
		if (!ray_t.surrounds(root))
		{
			root = (h + sqrtd) / a;
			if (!ray_t.contains(root))
				return false;
		}

		// 设置相交的hit_record的值
		rec.t = root;		 // the t value of intersection point
		rec.p = r.at(rec.t); // intersection point
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		rec.mat = mat; // the material of intersection point
		// ? 为什么用 normal
		get_sphere_uv(outward_normal, rec.u, rec.v); // 更新（u，v）

		return true;
	}

	// 实例化 virtual 函数
	aabb bounding_box() const override { return bbox; }

	// (x,y,z) -> (u,v)
	static void get_sphere_uv(const point3 &p, double &u, double &v)
	{
		auto theta = acos(-p.y());
		auto phi = atan2(-p.z(), p.x()) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}

	double pdf_value(const point3& origin, const vec3& direction) const override {
		// This method only works for stationary spheres.

		hit_record rec;
		if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
			return 0;

		auto cos_theta_max = sqrt(1 - radius * radius / (center1 - origin).length_squared());
		auto solid_angle = 2 * pi * (1 - cos_theta_max);

		return  1 / solid_angle;
	}

	vec3 random(const point3& origin) const override {
		vec3 direction = center1 - origin;
		auto distance_squared = direction.length_squared();
		onb uvw;
		uvw.build_from_w(direction);
		return uvw.local(random_to_sphere(radius, distance_squared));
	}
};

#endif