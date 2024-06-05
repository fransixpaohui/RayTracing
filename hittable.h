#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "aabb.h"

class material;

// object the ray can intersect with
class hit_record
{
public:
	point3 p;		 // ����������Ľ���
	vec3 normal;	 // ���㴦�ķ�����
	double t;		 // ���㴦��tֵ
	bool front_face; // �����Ƿ�������
	std::shared_ptr<material> mat;
	double u, v; // texture coordinate

	void set_face_normal(const ray &r, const vec3 &outward_normal)
	{
		// set the hit record normal vector
		// NOTE��the parameter 'outward_normal' is assumed to unit vector
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

// ������ཻ������Ļ���
class hittable
{
public:
	virtual ~hittable(){};
	// �жϹ��������Ƿ��ཻ
	virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const = 0;

	virtual aabb bounding_box() const = 0;
};

#endif