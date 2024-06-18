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

    virtual double pdf_value(const point3& origin, const vec3& direction) const {
        return 0.0;
    }

    virtual vec3 random(const point3& origin) const {
        return vec3(1, 0, 0);
    }
};

class translate :public hittable {
public:

	translate(shared_ptr<hittable> object, const vec3& offset) : object(object), offset(offset) {
		// aabb also need translation
		bbox = object->bounding_box() + offset;
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

		// move the ray backwards by the offset.
		ray offset_r(r.origin() - offset, r.direction(), r.time());

		// determine whether an intersection exists along the offset ray (an if so, where)
		if (!object->hit(offset_r, ray_t, rec)) return false;

		// move the intersection point forwards by the offset.
		rec.p += offset;

		return true;
	}

	aabb bounding_box() const override { return bbox; }

private:
	shared_ptr<hittable> object;
	vec3 offset;
	aabb bbox;
};

class rotate_y : public hittable {
public:

    rotate_y(shared_ptr<hittable> object, double angle) : object(object) {
        auto radians = degrees_to_radians(angle);
        sin_theta = sin(radians);
        cos_theta = cos(radians);
        bbox = object->bounding_box();

        point3 min(infinity, infinity, infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    auto x = i * bbox.interval_x.max + (1 - i) * bbox.interval_x.min;
                    auto y = j * bbox.interval_y.max + (1 - j) * bbox.interval_y.min;
                    auto z = k * bbox.interval_z.max + (1 - k) * bbox.interval_z.min;

                    auto newx = cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; c++) {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bbox = aabb(min, max);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Change the ray from world space to object space
        auto origin = r.origin();
        auto direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        // Determine whether an intersection exists in object space (and if so, where)
        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        // Change the intersection point from object space to world space
        auto p = rec.p;
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        // Change the normal from object space to world space
        auto normal = rec.normal;
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }
   
    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    aabb bbox;
};
#endif