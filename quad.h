#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"
#include "material.h"
#include "rtweekend.h"

class quad : public hittable
{
public:
	quad(const point3 &Q, const vec3 &u, const vec3 &v, shared_ptr<material> mat) : Q(Q), u(u), v(v), mat(mat)
	{

		auto n = cross(u, v);
		normal = unit_vector(n);
		D = dot(normal, Q);
		w = n / dot(n, n);

		set_bounding_box();
	}

	virtual void set_bounding_box()
	{
		// compute the bounding box of all four vertices
		auto bbox_diagonal1 = aabb(Q, Q + u + v);
		auto bbox_diagonal2 = aabb(Q + u, Q + v);
		bbox = aabb(bbox_diagonal1, bbox_diagonal2);
	}

	aabb bounding_box() const override
	{
		return bbox;
	}

	// ���ж�ray�Ƿ���plane�ཻ���ٿ����Ƿ���ƽ���ϵ��ı����ཻ
	bool hit(const ray &r, interval ray_t, hit_record &rec) const override
	{

		auto denom = dot(normal, r.direction());

		// no hit if the ray is parallel to the plane
		if (fabs(denom) < 1e-8)
			return false;

		//  return false if the hit point parameter t is outside the ray interval
		auto t = (D - dot(normal, r.origin())) / denom;
		if (!ray_t.contains(t))
			return false;

		// determin the hit point lies within the planar shape using its plane coordinates
		auto intersection = r.at(t);
		vec3 planar_hitpt_vector = intersection - Q;
		auto alpha = dot(w, cross(planar_hitpt_vector, v));
		auto beta = dot(w, cross(u, planar_hitpt_vector));

		if (!is_interior(alpha, beta, rec))
			return false;

		rec.t = t;
		rec.p = intersection;
		rec.mat = mat;
		rec.set_face_normal(r, normal);

		return true;
	}

	// ��������ϵ�������۳��ֱ���u,v�����alpha��beta������0~1֮��
	virtual bool is_interior(double a, double b, hit_record &rec) const
	{
		interval unit_interval = interval(0, 1);

		// given the hit point in plane coordinates,return false if it is outside the primitive,otherwise set the hit record UV coordinates and return true.

		if (!unit_interval.contains(a) || !unit_interval.contains(b))
			return false;

		rec.u = a;
		rec.v = b;
		return true;
	}

private:
	point3 Q;
	vec3 u, v;
	vec3 w;
	shared_ptr<material> mat;
	aabb bbox;
	vec3 normal;
	double D; // ƽ�淽�̵�Ax + By + Cz = D
};

#endif