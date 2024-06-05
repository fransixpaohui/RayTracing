#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

// sotre a list of hittable
class hittable_list : public hittable
{
private:
	aabb bbox;

public:
	std::vector<std::shared_ptr<hittable>> objects;

	hittable_list(){};

	hittable_list(std::shared_ptr<hittable> object) { add(object); }

	// 场景中加物品,并且更新aabb
	void add(std::shared_ptr<hittable> object)
	{
		objects.push_back(object);
		bbox = aabb(bbox, object->bounding_box());
	}

	void clear() { objects.clear(); }

	// 遍历所有hittable，判断是否有与ray相交的，并更新相应的hit_record
	bool hit(const ray &r, interval ray_t, hit_record &rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		auto closet_so_far = ray_t.max;

		for (const auto &object : objects)
		{
			if (object->hit(r, interval(ray_t.min, closet_so_far), temp_rec))
			{
				hit_anything = true;
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}
		return hit_anything;
	}

	aabb bounding_box() const override { return bbox; }
};

#endif