
#include <cmath>
#include <list>
#include <iostream>

#include "MCG_GFX_Lib.h"


struct HitData
{
	bool mHit;
	glm::vec3 mFirstIntersection;
};


class Ray
{
private:
	glm::vec3 mOrigin;
	glm::vec3 mDirection;

public:
	Ray(glm::vec3 origin, glm::vec3 direction)
	{
		mOrigin = origin;
		mDirection = direction;
	};
	~Ray() {};

	glm::vec3 GetOrigin()
	{
		return mOrigin;
	};
	glm::vec3 GetDirection()
	{
		return mDirection;
	};
};


class Sphere
{
private:
	glm::vec3 mCentre;
	glm::vec3 mColour;
	int mRadius;

public:
	Sphere(glm::vec3 centre, int radius, glm::vec3 colour)
	{
		mCentre = centre;
		mRadius = radius;
		mColour = colour;
	};
	~Sphere() {};

	glm::vec3 ShadePixel(Ray ray, glm::vec3 intersectionPoint)
	{
		// Returns colour
	};

	glm::ivec3 GetCentre()
	{
		return mCentre;
	};
	glm::vec3 GetColour()
	{
		return mColour;
	};
	int GetRadius()
	{
		return mRadius;
	};
};


// Outputs a vec3 to console (used for debugging)
void display_vec3(glm::vec3 vec)
{
	std::cout << vec.x << ", " << vec.y << ", " << vec.z << std::endl;
};


// Gets difference in direction
float get_direction_difference(glm::vec3 dir1, glm::vec3 dir2)
{
	glm::vec3 n_dir1 = glm::normalize(dir1);
	glm::vec3 n_dir2 = glm::normalize(dir2);

	float dif = glm::length(n_dir1 - n_dir2) / 2;

	return dif;
};


// Returns normal to the given sphere at given point
glm::vec3 get_normal_on_sphere(Sphere sphere, glm::vec3 queryPoint)
{
	glm::vec3 sphereCentre = sphere.GetCentre();
	glm::vec3 normal = queryPoint - sphereCentre;

	return glm::normalize(normal);
};


// Checks if the given point is inside the given sphere
bool check_inside_sphere(Sphere sphere, glm::vec3 queryPoint)
{
	glm::vec3 sphereCentre = sphere.GetCentre();

	int distanceToCentre = glm::length(sphereCentre - queryPoint);

	if (distanceToCentre <= sphere.GetRadius())
	{
		return true;
	};

	return false;
};


// Checks if the given point is ahead of the given ray
bool check_ahead_ray(Ray ray, glm::vec3 queryPoint)
{
	if (glm::normalize(ray.GetDirection()) == glm::normalize(queryPoint - ray.GetOrigin()))
	{
		return true;
	};

	return false;
};


// 𝒂 = Starting point of the line
// 𝑷 = Query point
// 𝒏 = Direction of the line
// Closest Point = 𝒂 + ((𝑷−𝒂)⋅𝒏)𝒏
// Return Closest Point
glm::vec3 get_closest_point_on_line(Ray line, glm::vec3 queryPoint)
{
	glm::vec3 a = line.GetOrigin();
	glm::vec3 n = line.GetDirection();
	glm::vec3 P = queryPoint;

	glm::vec3 closestPoint = a + (glm::dot((P - a), n)) * n;

	return closestPoint;
};


// 𝒂 = Starting point of the line
// 𝑷 = Centre of sphere
// 𝒏 = Direction of the line
// 𝑥 = Distance from closest point to intersection
// 𝑑 = Distance from closest point to centre of sphere
// Returns if hit and first intersection
HitData get_ray_sphere_intersection(Ray ray, Sphere sphere)
{
	glm::vec3 sphereCentre = sphere.GetCentre();
	int sphereRadius = sphere.GetRadius();

	glm::vec3 a = ray.GetOrigin();
	glm::vec3 n = ray.GetDirection();
	glm::vec3 P = sphereCentre;

	// Checks if ray origin is inside sphere, if so, treats as an error and returns no intersection
	if (check_inside_sphere(sphere, a))
	{
		return HitData{ false, glm::vec3(0,0,0) };
	};

	glm::vec3 closestPoint = get_closest_point_on_line(ray, sphereCentre);
	int d = glm::length(sphereCentre - closestPoint);
	int x = sqrt(pow(sphereRadius, 2) - pow(d, 2));

	// Checks if the closest point is ahead of the ray, if it's not, no intersection
	if (!check_ahead_ray(ray, closestPoint))
	{
		return HitData{ false, glm::vec3(0,0,0) };
	};

	if (d <= sphereRadius)
	{
		glm::vec3 firstIntersection = a + (glm::dot((P - a), n) - x) * n;
		return HitData{ true, firstIntersection };
	};

	return HitData{ false, glm::vec3(0,0,0) };
};


class Scene
{
private:
	glm::vec3 mLightDirection;
	std::list<Sphere> mSpheres;

public:
	Scene(glm::vec3 lightDirection) 
	{
		mLightDirection = lightDirection;
	};
	~Scene() {};

	void AddSphere(glm::vec3 centre, int radius, glm::vec3 colour)
	{
		Sphere newSphere(centre, radius, colour);
		mSpheres.push_back(newSphere);
	};

	float GetColourModifier(Sphere sphere, glm::vec3 intersectionPoint)
	{
		glm::vec3 sphereNormal = get_normal_on_sphere(sphere, intersectionPoint);

		return 1 - get_direction_difference(mLightDirection, sphereNormal);
	};

	glm::vec3 GetLightDirection()
	{
		return mLightDirection;
	};
	std::list<Sphere> GetSpheres()
	{
		return mSpheres;
	};
};


class RayTracer
{
private:
	Scene mCurrentScene;

public:
	RayTracer() : mCurrentScene(glm::vec3(1, -1, -1)) {};
	~RayTracer() {};

	glm::vec3 TraceRay(Ray ray)
	{
		std::list<Sphere> spheres = mCurrentScene.GetSpheres();

		// Cycle through list
		for (Sphere currentSphere : spheres)
		{
			// Check for collision
			HitData currentHitData = get_ray_sphere_intersection(ray, currentSphere);

			if (currentHitData.mHit)
			{
				float colourModifier = mCurrentScene.GetColourModifier(currentSphere, currentHitData.mFirstIntersection);

				// If collision, return red
				return currentSphere.GetColour() * colourModifier;
			};
		}

		// If no collision return black
		return glm::vec3(0, 0, 0);
	};
	void SetScene(Scene scene)
	{
		mCurrentScene = scene;
	};
};


class Camera
{
private:
	glm::ivec2 mWindowSize;
	glm::ivec2 mWindowCentre;
	glm::ivec2 mViewingSize;

	float mXViewMultiplier;
	float mYViewMultiplier;
	float mXViewOffset;
	float mYViewOffset;

public:
	Camera(glm::ivec2 windowSize, glm::ivec2 viewingSize)
	{
		mWindowSize = windowSize;
		mWindowCentre = windowSize / 2;
		mViewingSize = viewingSize;

		mXViewMultiplier = mViewingSize.x / mWindowSize.x;
		mYViewMultiplier = mViewingSize.y / mWindowSize.y;
		mXViewOffset = (mViewingSize.x - mWindowSize.x) / 2;
		mYViewOffset = (mViewingSize.y - mWindowSize.y) / 2;
	};
	~Camera() {};

	Ray GetRay(glm::ivec2 pixelPosition)
	{
		glm::vec3 source;
		glm::vec3 lead;

		source.x = (float)pixelPosition.x;
		source.y = (float)pixelPosition.y;
		source.z = -1.f;

		lead.x = (float)(pixelPosition.x * mXViewMultiplier - mXViewOffset);
		lead.y = (float)(pixelPosition.y * mYViewMultiplier - mYViewOffset);
		lead.z = 1.f;

		Ray ray(source, glm::normalize(lead - source));

		return ray;
	};
};


int main( int argc, char *argv[] )
{
	// Variable for storing window dimensions
	glm::ivec2 windowSize( 640, 480 );

	// Call MCG::Init to initialise and create your window
	// Tell it what size you want the window to be
	if( !MCG::Init( windowSize ) )
	{
		// We must check if something went wrong
		// (this is very unlikely)
		return -1;
	}

	// Sets every pixel to the same colour
	// parameters are RGB, numbers are from 0 to 1
	MCG::SetBackground( glm::vec3(0,0,0) );

	// Preparing a position to draw a pixel
	glm::ivec2 pixelPosition = windowSize / 2;

	// Preparing a colour to draw
	// Colours are RGB, each value ranges between 0 and 1
	glm::vec3 pixelColour( 1, 0, 0 );


	// Draws a single pixel at the specified coordinates in the specified colour!
	MCG::DrawPixel( pixelPosition, pixelColour );

	// Do any other DrawPixel calls here
	// ...
	Camera camera(windowSize, windowSize);

	Scene scene(glm::vec3(1, -1, -1));
	scene.AddSphere(glm::ivec3(100, 100, 20), 20, glm::vec3(1, 0, 0));
	scene.AddSphere(glm::ivec3(300, 300, 30), 30, glm::vec3(0, 0, 1));
	scene.AddSphere(glm::ivec3(220, 220, 200), 160, glm::vec3(0, 1, 0));

	RayTracer rayTracer;
	rayTracer.SetScene(scene);

	for (int x = 0; x < windowSize.x; x++)
	{
		for (int y = 0; y < windowSize.y; y++)
		{
			glm::ivec2 pixelPosition(x, y);

			Ray currentRay = camera.GetRay(pixelPosition);

			glm::vec3 pixelColour = rayTracer.TraceRay(currentRay);

			MCG::DrawPixel(pixelPosition, pixelColour);
		};
	};

	// Displays drawing to screen and holds until user closes window
	// You must call this after all your drawing calls
	// Program will exit after this line
	return MCG::ShowAndHold();
}
