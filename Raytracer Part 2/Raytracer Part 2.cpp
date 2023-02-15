/* Created by: Miguel R. Kunkle
* Date: 2/9/23
* Second step of a simple raytracer.
* Added light and diffuse reflection to make the spheres look like spheres
*/

#include <SDL.h>
#include <iostream>
#include <cmath>
#include <array>
#undef main

//-------------------------------
// Linear Algebra functions
// ------------------------------

//dot product of two 3d vectors
double DotProduct(std::array<double, 3> v1, std::array<double, 3> v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

//computes v1 - v2
std::array<double, 3> subtract(std::array<double, 3> v1, std::array<double, 3> v2)
{
	std::array<double, 3> a;
	for (int i = 0; i < 3; i++)
	{
		a[i] = v1[i] - v2[i];
	}

	return a;
}

//computes v1 + v2
std::array<double, 3> add(std::array<double, 3> v1, std::array<double, 3> v2)
{
	std::array<double, 3> b;
	for (int i = 0; i < 3; i++)
	{
		b[i] = v1[i] + v2[i];
	}

	return b;
}

//length of a 3D vector
double length(std::array<double, 3> vec)
{
	return std::sqrt(DotProduct(vec, vec));
}

//scales a vector by k
std::array<double, 3> multiply(double k, std::array<double, 3> vec)
{
	std::array<double, 3> product;

	for (int i = 0; i < 3; i++)
	{
		product[i] = vec[i] * k;
	}

	return product;
}

// Clamps a color to the canonical color range.
//min makes sure value doesnt go over 255(highest color range), max makes sure value doesnt go below 0
std::array<double, 3> clamp(std::array<double, 3> vec)
{
	std::array<double, 3> clampColor;
	for (int i = 0; i < 3; i++)
	{
		clampColor[i] = std::fmin(255, std::fmax(0, vec[i]));
	}

	return clampColor;
}


//------------------------------------------------------------
// Raytracer, now with D I F F U S E I L L U M I N A T I O N
// -----------------------------------------------------------

//Scene Setup
double viewport_size = 1;
double  projection_plane_z = 1;
std::array<double, 3> camera_position = { 0, 0, 0 };
std::array<double, 3> background_color = { 255, 255, 255 };

//canvas width and height
const double WIDTH = 600;
const double HEIGHT = 600;


//--------
//Spheres
//--------

//To create spheres
struct Sphere
{
	std::array<double, 3> center;
	double radius;
	std::array<double, 3> color;
};

//create spheres
Sphere sphere1{ {0, -1, 3}, 1, {255, 0, 0} };
Sphere sphere2{ {2, 0, 4}, 1,{0, 0, 255 } };
Sphere sphere3{ {-2, 0, 4}, 1,{0, 255, 0 } };
Sphere sphere4{ {0,-5001,0}, 5000, {255,0,255} };

Sphere* spheres[] = { &sphere1, &sphere2, &sphere3, &sphere4 };

//-------
//Lights
//-------

//To create lights
struct Light
{
	std::string ltype;
	double intensity;
	std::array<double, 3> position;
};


//Light array
Light light1{ "AMBIENT", 0.2};
Light light2{ "POINT", 0.6, {2, 1, 0} };
Light light3{ "DIRECTIONAL", 0.2, {1, 4, 4} };

Light* lights[] = { &light1, &light2, &light3 };




//define "infinity"
const double INFIN = 2147483647;

//Conerts 2D canvas coordinates to 3D viewport coordinates
std::array<double, 3> CanvasToViewport(std::array<double, 2>  p2d)
{
	std::array<double, 3> a;

	a[0] = p2d[0] * viewport_size / WIDTH;
	a[1] = p2d[1] * viewport_size / HEIGHT;
	a[2] = projection_plane_z;

	return a;
}

// Computes the intersection of a ray and a sphere. Returns the values of t for the intersections.
//Camera position is origin
//Direction is what CanvasToViewport returns

std::array<double, 2> IntersectRaySphere(std::array<double, 3> origin, std::array<double, 3> direction, Sphere* sphere)
{
	std::array<double, 3> oc = subtract(origin, sphere->center);

	double k1 = DotProduct(direction, direction);
	double k2 = 2 * DotProduct(oc, direction);
	double k3 = DotProduct(oc, oc) - sphere->radius * sphere->radius;

	double discriminant = k2 * k2 - 4 * k1 * k3;
	if (discriminant < 0)
	{
		std::array<double, 2> infinity = { INFIN, INFIN };
		return infinity;
	}

	double t1 = (-k2 + std::sqrt(discriminant)) / (2 * k1);
	double t2 = (-k2 - std::sqrt(discriminant)) / (2 * k1);

	std::array<double, 2> t = { t1,t2 };

	return t;
}

//Its in the name
double ComputeLighting(std::array<double, 3> point, std::array<double, 3> normal)
{
	double intensity = 0;
	double length_n = length(normal); //Verifying the length of our normal is 1.

	//amount of lights is hard coded to 3
	for (int i = 0; i < 3; i++)
	{
		Light* light = lights[i];


		if (light->ltype == "AMBIENT")
		{
			intensity += light->intensity;
		}

		else
		{
			std::array<double, 3>  vec1;
			if (light->ltype == "POINT")
			{
				vec1 = subtract(light->position, point);
			}

			else //This is directional light
			{
				vec1 = light->position;
			}

			double n_dot_1 = DotProduct(normal, vec1);
			if (n_dot_1 > 0)
			{
				intensity += light->intensity * n_dot_1 / (length_n * length(vec1));
			}
		}

		
	}
	return intensity;
}

//Traces a ray against the set of spheres in the scene.
//min_t is 1, max_t is infinity.
std::array<double, 3> TraceRay(std::array<double, 3>  origin, std::array<double, 3> direction, double min_t, double max_t)
{
	double closest_t = INFIN;
	Sphere* closest_sphere = nullptr;

	//4 IS HARD CODED FIX LATER LOVE YOU MWAH
	for (int i = 0; i < 4; i++)
	{
		std::array<double, 2> ts = IntersectRaySphere(origin, direction, spheres[i]);

		if (ts[0] < closest_t && min_t < ts[0] && ts[0] < max_t)
		{
			closest_t = ts[0];
			closest_sphere = spheres[i];
		}

		if (ts[1] < closest_t && min_t < ts[1] && ts[1] < max_t)
		{
			closest_t = ts[1];
			closest_sphere = spheres[i];
		}
	}
	if (closest_sphere == nullptr)
	{
		return background_color;
	}

	std::array<double, 3> point = add(origin, multiply(closest_t, direction));
	std::array<double, 3> normal = subtract(point, closest_sphere->center);

	normal = multiply(1.0 / length(normal), normal);


	return multiply(ComputeLighting(point, normal), closest_sphere->color);
}

void PutPixel(int x, int y, std::array<double, 3> color, SDL_Renderer* renderer)
{
	x = WIDTH / 2 + x;
	y = HEIGHT / 2 - y - 1;

	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
	{
		return;
	}

	SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
	SDL_RenderDrawPoint(renderer, x, y);
}

int main()
{
	//SDL Making Window
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_RenderClear(renderer);

	for (double x = -WIDTH / 2; x < WIDTH / 2; x++)
	{
		for (double y = -HEIGHT / 2; y < HEIGHT / 2; y++)
		{
			std::array<double, 2> XY = { x,y };
			std::array<double, 3> direction = CanvasToViewport(XY);
			std::array<double, 3> color = TraceRay(camera_position, direction, 1, INFIN);
			PutPixel(x, y, clamp(color), renderer);
		}
	}

	SDL_RenderPresent(renderer);
	
	//delay is currently 3 seconds
	SDL_Delay(4000);
	return 0;
}

