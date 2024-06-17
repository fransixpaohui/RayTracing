#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "hittable_list.h"
#include "material.h"
#include "pdf.h"

#include <fstream>
#include <omp.h>

// consolidate the camera and scene-render code
class camera
{
private:
	int image_height;			// Rendered image height
	point3 center;				// Camera center
	point3 pixel00_loc;			// Location of pixel 0, 0
	vec3 pixel_delta_u;			// Offset to pixel to the right
	vec3 pixel_delta_v;			// Offset to pixel below
	double pixel_samples_scale; // color scale factor of pixel samples
	vec3 u, v, w;				// Camera frame basis vectors
	vec3 defocus_disk_u;		// Defocus disk horizontal radius
	vec3 defocus_disk_v;		// Defocus disk vertical radius
	int    sqrt_spp;             // Square root of number of samples per pixel
	double recip_sqrt_spp;       // 1 / sqrt_spp

	void initialize()
	{
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		sqrt_spp = int(sqrt(samples_per_pixel));
		pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
		recip_sqrt_spp = 1.0 / sqrt_spp;

		center = lookfrom;

		// Determine viewport dimensions.
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0 * h * focus_dist;
		auto viewport_width = viewport_height * (double(image_width) / image_height);

		// calculate u,v,w unit basis vectors for the camera coordinate frame
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		auto viewport_u = viewport_width * u;
		auto viewport_v = viewport_height * -v;

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left =
			center - focus_dist * w - viewport_u / 2 - viewport_v / 2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

		// Calculate the camera defocus disk basis vectors
		auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	}

public:
	double aspect_ratio = 1.0;	// ratio of image width over height
	int image_width = 100;		// rendered image width in pixel count
	int samples_per_pixel = 10; // count of random samples for each pixel
	int max_depth = 10;			// Maximum number of ray bounce int the scene
	color background;			// Scene background color

	double vfov = 90;				   // vertial view angle (field of view)
	point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
	point3 lookat = point3(0, 0, -1);  // Point camera is looking at
	vec3 vup = vec3(0, 1, 0);		   // Camera-relative "up" direction

	double defocus_angle = 0; // variation angle of rays through each pixel
	double focus_dist = 10;	  // distance from camera lookfrom point to plane of perfect focus

	void render(const hittable &world)
	{
		initialize();

		// 建立一个图像数组
		std::vector<std::vector<color>> colorbuffer(image_height);
		for (int i = 0; i < image_height; i++)
		{
			colorbuffer[i].resize(image_width);
		}

		int scan = 0;

		omp_set_num_threads(50); // 开启线程数
		#pragma omp parallel for 
		for (int j = 0; j < image_height; j++)
		{
			std::clog << "\rScanlines remaining: " << (image_height - scan) << ' ' << std::flush;
			#pragma omp parallel for
			for (int i = 0; i < image_width; i++)
			{
				color pixel_color(0, 0, 0);
				for (int s_j = 0; s_j < sqrt_spp; s_j++) {
					for (int s_i = 0; s_i < sqrt_spp; s_i++) {
						ray r = get_ray(i, j, s_i, s_j);
						pixel_color += ray_color(r, max_depth, world);
					}
				}
				write_color(colorbuffer, i, j, pixel_samples_scale * pixel_color);
			}
			scan++;
		}
		std::clog << "\rDone.                 \n";

		// 将图像数据写入ppm文件
		std::ofstream OutImage;
		OutImage.open("Image.ppm");

		OutImage << "P3\n"
				 << image_width << ' ' << image_height << "\n255\n";

		for (int j = 0; j < image_height; j++)
		{
			for (int i = 0; i < image_width; i++)
			{
				OutImage << colorbuffer[j][i][0] << ' ' << colorbuffer[j][i][1] << ' ' << colorbuffer[j][i][2] << '\n';
			}
		}
		OutImage.close();
	}

	// 在像素点（i，j）附近采样并获取ray
	ray get_ray(int i, int j,int s_i,int s_j) const
	{
		// Construct a camera ray originating from the defocus disk and directed at a randomly
	   // sampled point around the pixel location i, j for stratified sample square s_i, s_j.
		auto offset = sample_square_stratified(s_i, s_j);
		auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		auto ray_time = random_double();

		return ray(ray_origin, ray_direction, ray_time);
	}

	vec3 sample_square_stratified(int s_i, int s_j) const {
		// Returns the vector to a random point in the square sub-pixel specified by grid
		// indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].
		auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
		auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

		return vec3(px, py, 0);
	}

	vec3 sample_square() const
	{
		return vec3(random_double() - 0.5, random_double() - 0.5, 0);
	}

	point3 defocus_disk_sample() const
	{
		// return a random point in the camera defocus disk.
		auto p = random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}

	color ray_color(const ray &r, int depth, const hittable &world)
	{
		// if we've exceeded the ray bounce limit, no more light is gathered.
		if (depth <= 0)
			return color(0, 0, 0);

		// hit record
		hit_record rec;

		// if the ray hits noting return teh background color
		if (!world.hit(r, interval(0.001, infinity), rec)) return background;

		ray scattered;
		color attenuation;
		double pdf_val;
		color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

		if (!rec.mat->scatter(r, rec, attenuation, scattered, pdf_val)) return color_from_emission;

		cosine_pdf surface_pdf(rec.normal);
		scattered = ray(rec.p, surface_pdf.generate(), r.time());
		pdf_val = surface_pdf.value(scattered.direction());

		double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

		color color_from_scatter =
			(attenuation * scattering_pdf * ray_color(scattered, depth - 1, world)) / pdf_val;

		return color_from_emission + color_from_scatter;
	}
};

#endif