#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable_list.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"

class scatter_record {
public:
	color attenuation;
	shared_ptr<pdf> pdf_ptr;
	bool skip_pdf;
	ray skip_pdf_ray;
};

class material
{
public:
	virtual ~material() = default;

	virtual bool scatter(const ray &r_in, const hit_record &rec, scatter_record& srec) const
	{
		return false;
	}

	virtual color emitted(
		const ray& r_in, const hit_record& rec, double u, double v, const point3& p
	) const {
		return color(0, 0, 0);
	}

	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
		const {
		return 0;
	}
};

class lambertian : public material
{
public:
	lambertian(const color &albedo) : tex(make_shared<solid_color>(albedo)) {}

	lambertian(shared_ptr<texture> tex) : tex(tex) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
		srec.attenuation = tex->value(rec.u, rec.v, rec.p);
		srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
		srec.skip_pdf = false;
		return true;
	}

	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
		auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
		return cosine < 0 ? 0 : cosine / pi;
	}

private:
	shared_ptr<texture> tex;
};

// the metal material just reflect rays
class metal : public material
{
public:
	metal(const color &albedo, const double &fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = unit_vector(reflected) + fuzz * (random_unit_vec());

		srec.attenuation = albedo;
		srec.pdf_ptr = nullptr;
		srec.skip_pdf = true;
		srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());

		return true;
	}

private:
	color albedo;
	double fuzz;
};

class dielectric : public material
{
public:
	dielectric(double refraction_index) : refraction_index(refraction_index) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		srec.attenuation = color(1.0, 1.0, 1.0);
		srec.pdf_ptr = nullptr;
		srec.skip_pdf = true;
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

		srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());

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
	diffuse_light(shared_ptr<texture> tex) : emit(tex) {}

	diffuse_light(const color &emit) : emit(make_shared<solid_color>(emit)) {}

	color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p)
		const override {
		if (!rec.front_face)
			return color(0, 0, 0);
		return emit->value(u, v, p);
	}

private:
	shared_ptr<texture> emit;
};

class isotropic : public material {
public:
	isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
	isotropic(shared_ptr<texture> tex) : tex(tex) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
		srec.attenuation = tex->value(rec.u, rec.v, rec.p);
		srec.pdf_ptr = make_shared<sphere_pdf>();
		srec.skip_pdf = false;
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