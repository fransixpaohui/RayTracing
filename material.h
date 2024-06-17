#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable_list.h"
#include "texture.h"
#include "onb.h"

class material
{
public:
	virtual ~material() = default;

	// 默认材质无法发生scatter
	virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered, double& pdf) const
	{
		return false;
	}

	// 若不是光源，则 color_from_emission 是 color(0,0,0);
	virtual color emitted(double u, double v, const point3 &p) const
	{
		return color(0, 0, 0);
	}

	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const {
		return 0;
	}
};

// 散射ray
class lambertian : public material
{
public:
	lambertian(const color &albedo) : tex(make_shared<solid_color>(albedo)) {}

	lambertian(shared_ptr<texture> tex) : tex(tex) {}

	// attenuation 表示经过该次scatter后得到的颜色
	bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
	) const override {
		onb uvw;
		uvw.build_from_w(rec.normal);
		auto scatter_direction = uvw.local(random_cosine_direction());

		scattered = ray(rec.p, unit_vector(scatter_direction), r_in.time());
		attenuation = tex->value(rec.u, rec.v, rec.p);
		pdf = dot(uvw.w(), scattered.direction()) / pi;
		return true;
	}

	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
		return 1 / (2 * pi);
	}

private:
	shared_ptr<texture> tex;
};

// the metal material just reflect rays
class metal : public material
{
public:
	metal(const color &albedo, const double &fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

	bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scatterd, double& pdf) const override
	{
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = unit_vector(reflected) + fuzz * (random_unit_vec());
		scatterd = ray(rec.p, reflected, r_in.time());
		attenuation = albedo;
		return (dot(scatterd.direction(), rec.normal) > 0);
	}

private:
	color albedo;
	double fuzz;
};

class dielectric : public material
{
public:
	dielectric(double refraction_index) : refraction_index(refraction_index) {}

	bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered, double& pdf) const override
	{
		attenuation = color(1.0, 1.0, 1.0);
		double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;
		vec3 unit_direction = unit_vector(r_in.direction());

		double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = ri * sin_theta > 1.0;

		vec3 direction;
		if (cannot_refract || reflectance(cos_theta, ri) > random_double())
			direction = reflect(unit_direction, rec.normal);
		else
			direction = refract(unit_direction, rec.normal, ri);

		scattered = ray(rec.p, direction, r_in.time());

		return true;
	}

private:
	double refraction_index;

	static double reflectance(double cosine, double refraction_index)
	{
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - refraction_index) / (1 + refraction_index);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

// lights : it just tell the ray what color it is and performs no reflection
class diffuse_light : public material
{
public:
	diffuse_light(shared_ptr<texture> tex) : tex(tex) {}

	diffuse_light(const color &emit) : tex(make_shared<solid_color>(emit)) {}

	color emitted(double u, double v, const point3 &p) const override
	{
		return tex->value(u, v, p);
	}

private:
	shared_ptr<texture> tex;
};

class isotropic : public material {
public:
	isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
	isotropic(shared_ptr<texture> tex) : tex(tex) {}

	bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, double& pdf
	) const override {
		attenuation = tex->value(rec.u, rec.v, rec.p);
		scattered = ray(rec.p, random_unit_vec(), r_in.time());
		pdf = 1 / (4 * pi);
		return true;
	}

	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const override {
		return 1 / (4 * pi);
	}

private:
	shared_ptr<texture> tex;
};

#endif
