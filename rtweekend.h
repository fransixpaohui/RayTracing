#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <limits>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;
using std::vector;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degress)
{
	return degress * pi / 180.0;
}

inline double random_double()
{
	// return a random real in [0,1)
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
	// return a random real in [min,max)
	return min + (max - min) * random_double();
}

inline int random_int(int min, int max)
{
	return int(random_double(min, max + 1));
}

// Common Headers

#include "color.h"
#include "ray.h"
#include "interval.h"
#include "vec3.h"

#endif