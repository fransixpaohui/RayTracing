#include "rtweekend.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include <time.h>
#include "bvh.h"

void bounsing_shperes()
{
	// World
	hittable_list world;

	auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
	world.add(make_shared<sphere>(point3(0,-1000,0),1000,make_shared<lambertian>(checker)));

	//auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	//world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	world = hittable_list(make_shared<bvh_node>(world));

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 200;
	cam.samples_per_pixel = 50;
	cam.max_depth = 10;

	cam.vfov = 20;
	cam.lookfrom = point3(13, 2, 3);
	cam.lookat = point3(0, 0, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0.6;
	cam.focus_dist = 10.0;

	cam.render(world);
}

void checkered_spheres() {

	hittable_list world;

	auto checker = make_shared<checker_texture>(0.32, color(.8, .6, .1), color(.9, .9, .9));

	world.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
	world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 400;
	cam.samples_per_pixel = 100;
	cam.max_depth = 50;

	cam.vfov = 20;
	cam.lookfrom = point3(13, 2, 3);
	cam.lookat = point3(0, 0, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0;

	cam.render(world);
}

void earth() {
	auto earth_texture = make_shared<image_texture>("earthmap.jpg"); // 将图片变成纹理类
	auto earth_surface = make_shared<lambertian>(earth_texture);
	auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface); // stationary sphere

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 800;
	cam.samples_per_pixel = 100;
	cam.max_depth = 50;

	cam.vfov = 20;
	cam.lookfrom = point3(0, 0, 12);
	cam.lookat = point3(0, 0, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0;

	cam.render(hittable_list(globe));
}

int main() {
	clock_t start, end;
	start = clock();

	switch (3) {
	case 1:bounsing_shperes();
		break;
	case 2:checkered_spheres();
		break;
	case 3:earth();
		break;
	}

	end = clock();
	int time = (end - start) / CLOCKS_PER_SEC;
	int h = time / (60 * 60);
	time = time % (60 * 60);
	int min = time / 60;
	time = time % 60;
	std::cout << "\n"
		<< h << "小时 " << min << "分钟 " << time << "秒";
}