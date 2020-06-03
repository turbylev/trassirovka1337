#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "geometry.h"
using namespace std;


struct Ball {
	Ball(const Vec3f& color) : diffuse_color(color) {}
	Ball() : diffuse_color() {}
	Vec3f diffuse_color;
	
};
struct Light {
	Light(const Vec3f& p, const float i) : position(p), intensity(i) {}
	Vec3f position;
	float intensity;
	
};

struct Sphere {
	Vec3f center;
	float radius;
	Ball ball;
	
	Sphere(const Vec3f& c, const float r, const Ball& m) : center(c), radius(r), ball(m) {}

	bool ray_intersect(const Vec3f& dir, float& t0) const {
		Vec3f L = center;
		float tca = L * dir;
		float d2 = L * L - tca * tca;
		if (d2 > radius* radius) return false;
		float thc = sqrtf(radius * radius - d2);
		t0 = tca - thc;
		float t1 = tca + thc;
		if (t0 < 0) t0 = t1;
		if (t0 < 0) return false;
		return true;
		//cout << "2";
	}
};


bool scene_intersect(const Vec3f& dir, const vector<Sphere>& spheres, Vec3f& hit, Vec3f& N, Ball& ball) {
	float spheres_dist = numeric_limits<float>::max();
	for (size_t i = 0; i < spheres.size(); i++) {
		float dist_i;
		if (spheres[i].ray_intersect(dir, dist_i) && dist_i < spheres_dist) {
			spheres_dist = dist_i;
			hit = dir * dist_i;
			N = (hit - spheres[i].center).normalize();
			ball = spheres[i].ball;
		}
		
	}
	return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const vector<Sphere>& spheres, const vector<Light>& lights, size_t depth = 0) {
	Vec3f point, N;
	Ball ball;
	//cout << "1";
	if (depth > 4 || !scene_intersect(dir, spheres, point, N, ball)) {
		return Vec3f( 0.160, 0.160, 0.160); 
	}


	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (size_t i = 0; i < lights.size(); i++) {
		Vec3f light_dir = (lights[i].position - point).normalize();
		float light_distance = (lights[i].position - point).norm();

		Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; 
		Vec3f shadow_pt, shadow_N;
		Ball tmpball;
		if (scene_intersect(light_dir, spheres, shadow_pt, shadow_N, tmpball) && (shadow_pt - shadow_orig).norm() < light_distance)
			continue;

		diffuse_light_intensity += lights[i].intensity * max(0.f, light_dir * N);
		//cout << "4";
	}
	return ball.diffuse_color * diffuse_light_intensity + Vec3f(1., 1., 1.) * specular_light_intensity;
}

void render(const vector<Sphere>& spheres, const vector<Light>& lights) {
	const int   width = 1920;
	const int   height = 1080;
	const float fov = M_PI / 3.;
	vector<Vec3f> framebuffer(width * height);
	//cout << "1";
#pragma omp parallel for
	for (size_t j = 0; j < height; j++) { 
		for (size_t i = 0; i < width; i++) {
			float dir_x = (i + 0.5) - width / 2.;
			float dir_y = -(j + 0.5) + height / 2.;    
			float dir_z = -height / (2. * tan(fov / 2.));
			framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), Vec3f(dir_x, dir_y, dir_z).normalize(), spheres, lights);
		}
	}

	ofstream ofs; 
	ofs.open("./grapf.ppm", ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (size_t i = 0; i < height * width; ++i) {
		Vec3f& c = framebuffer[i];
		float max = std::max(c[0], std::max(c[1], c[2]));
		if (max > 1) c = c * (1. / max);
		for (size_t j = 0; j < 3; j++) {
			ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
		}
	}
	//cout << "5";
	ofs.close();
}

int main() {
	Ball  yran(Vec3f(0.210, 0.74, 0.67));

	Ball neptun(Vec3f(0.4, 0.5, 0.8));


	vector<Sphere> spheres;
	spheres.push_back(Sphere(Vec3f(2, -0.5, -18), 2.5, yran));
	spheres.push_back(Sphere(Vec3f(-3, -2, -10), 2.1, neptun));

	

	vector<Light>  lights;
	lights.push_back(Light(Vec3f(-10, -10, 10), 1.5));

	render(spheres, lights);
	//cout << "6";
	return 0;
}
