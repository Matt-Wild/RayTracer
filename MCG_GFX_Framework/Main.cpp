
#include <cmath>
#include <list>
#include <iostream>
#include <algorithm>

#include "MCG_GFX_Lib.h"

// HitData prototype
struct HitData;

// Class prototypes
class Ray;
class Sphere;
class Scene;
class RayTracer;
class Camera;

// Function prototypes
void display_vec3(glm::vec3 vec);
float area_of_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
bool point_inside_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int px, int py);
HitData get_ray_triangle_intersection(Ray ray, float z, glm::vec2 pointA, glm::vec2 pointB, glm::vec2 pointC);
HitData get_ray_rectangle_intersection(Ray ray, glm::vec3 rect_pos, float rect_width, float rect_height);
HitData get_ray_circle_intersection(Ray ray, glm::vec3 circle_pos, float circle_radius);
glm::vec3 get_point_at_z(Ray ray, float z);
float get_direction_difference(glm::vec3 dir1, glm::vec3 dir2);
glm::vec3 get_normal_on_sphere(Sphere sphere, glm::vec3 queryPoint);
bool check_inside_sphere(Sphere sphere, glm::vec3 queryPoint);
bool check_ahead_ray(Ray ray, glm::vec3 queryPoint);
glm::vec3 get_closest_point_on_line(Ray line, glm::vec3 queryPoint);
HitData get_ray_sphere_intersection(Ray ray, Sphere sphere);
float get_length_between_points(glm::vec3 point1, glm::vec3 point2);


struct HitData
{
	// Stores if a collision has been detected
	bool mHit;
	// Stores point of collision
	glm::vec3 mFirstIntersection;
};


class Ray
{
private:
	// Stores where the ray began
	glm::vec3 mOrigin;
	// Stores the direction of the ray
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


class BaseShape
{
protected:
	// Stores the shape's position
	glm::vec3 mPos;
	// Stores the shape's colour
	glm::vec3 mColour;

public:
	BaseShape(glm::vec3 pos, glm::vec3 colour)
	{
		mPos = pos;
		mColour = colour;
	};

	// Gets the colour modifier for the pixel (adjusts brightness based on lighting)
	virtual float GetColourModifier(glm::vec3 lightDirection, glm::vec3 intersectionPoint) { return 0; };
	// Gets data on if the given ray collides with the shape
	virtual HitData GetHit(Ray ray) { return HitData{ false, glm::vec3(0, 0, 0) }; };

	glm::vec3 GetPos()
	{
		return mPos;
	};
	glm::vec3 GetColour()
	{
		return mColour;
	};
};


class Triangle : public BaseShape
{
private:
	// Stores the 3 corner points of the triangle
	glm::vec2 mAPos, mBPos, mCPos;

public:
	Triangle(float z, glm::vec2 aPos, glm::vec2 bPos, glm::vec2 cPos, glm::vec3 colour)
		: BaseShape(glm::vec3(0, 0, z), colour)
	{
		mAPos = aPos;
		mBPos = bPos;
		mCPos = cPos;
	};

	float GetColourModifier(glm::vec3 lightDirection, glm::vec3 intersectionPoint)
	{
		// Basic colour modifier for 2D objects
		return pow(1 - get_direction_difference(lightDirection, glm::vec3(0, 0, -1)), 2);
	};
	HitData GetHit(Ray ray)
	{
		// Allows for the triangle's points to be moved evenly based on shape position
		glm::vec2 posAdj(mPos.x, mPos.y);

		// Gets intersection data
		return get_ray_triangle_intersection(ray, mPos.z, mAPos + posAdj, mBPos + posAdj, mCPos + posAdj);
	};
};


class Rectangle : public BaseShape
{
private:
	// Stores rectangle dimensions
	float mWidth, mHeight;

public:
	Rectangle(glm::vec3 pos, float width, float height, glm::vec3 colour)
		: BaseShape(pos, colour)
	{
		mWidth = width;
		mHeight = height;
	};

	float GetColourModifier(glm::vec3 lightDirection, glm::vec3 intersectionPoint)
	{
		// Basic colour modifier for 2D objects
		return pow(1 - get_direction_difference(lightDirection, glm::vec3(0, 0, -1)), 2);
	};
	HitData GetHit(Ray ray)
	{
		// Gets intersection data
		return get_ray_rectangle_intersection(ray, mPos, mWidth, mHeight);
	};
};


class Circle : public BaseShape
{
private:
	// Stores circle radius
	float mRadius;

public:
	Circle(glm::vec3 pos, float radius, glm::vec3 colour)
		: BaseShape(pos, colour)
	{
		mRadius = radius;
	};

	float GetColourModifier(glm::vec3 lightDirection, glm::vec3 intersectionPoint)
	{
		// Basic colour modifier for 2D objects
		return pow(1 - get_direction_difference(lightDirection, glm::vec3(0, 0, -1)), 2);
	};
	HitData GetHit(Ray ray)
	{
		// Gets intersection data
		return get_ray_circle_intersection(ray, mPos, mRadius);
	};
};


class Sphere : public BaseShape
{
private:
	// Stores sphere radius
	float mRadius;

public:
	Sphere(glm::vec3 pos, float radius, glm::vec3 colour)
		: BaseShape(pos, colour)
	{
		mRadius = radius;
	};

	float GetColourModifier(glm::vec3 lightDirection, glm::vec3 intersectionPoint) 
	{
		// Get normal to the sphere at intersection point
		glm::vec3 sphereNormal = get_normal_on_sphere(*this, intersectionPoint);

		// Gets colour modifier based on similarity of normal and light direction
		return pow(1 - get_direction_difference(lightDirection, sphereNormal), 2);
	};
	HitData GetHit(Ray ray)
	{
		// Gets intersection data
		return get_ray_sphere_intersection(ray, *this);
	};
	int GetRadius()
	{
		return mRadius;
	};
};


class Scene
{
private:
	// Stores the vector direction for lighting
	glm::vec3 mLightDirection;
	// List of shapes to render
	std::list<BaseShape*> mShapes;

public:
	Scene(glm::vec3 lightDirection) 
	{
		mLightDirection = lightDirection;
	};
	~Scene() {};

	// Adds sphere to shapes list
	void AddSphere(glm::vec3 centre, float radius, glm::vec3 colour)
	{
		mShapes.push_back(new Sphere(centre, radius, colour));
	};
	// Adds rectangle to shapes list
	void AddRectangle(glm::vec3 centre, float width, float height, glm::vec3 colour)
	{
		mShapes.push_back(new Rectangle(centre, width, height, colour));
	};
	// Addes circle to shapes list
	void AddCircle(glm::vec3 centre, float radius, glm::vec3 colour)
	{
		mShapes.push_back(new Circle(centre, radius, colour));
	};
	// Addes triangle to shapes list
	void AddTriangle(float z, glm::vec2 pointA, glm::vec2 pointB, glm::vec2 pointC, glm::vec3 colour)
	{
		mShapes.push_back(new Triangle(z, pointA, pointB, pointC, colour));
	};

	// Gets colour modifer from specific shape
	float GetColourModifier(BaseShape* shape, glm::vec3 intersectionPoint)
	{
		return shape->GetColourModifier(mLightDirection, intersectionPoint);
	};

	glm::vec3 GetLightDirection()
	{
		return mLightDirection;
	};
	std::list<BaseShape*> GetShapes()
	{
		return mShapes;
	};
};


class RayTracer
{
private:
	// Stores current scene
	Scene mCurrentScene;

public:
	RayTracer() : mCurrentScene(glm::vec3(1, -1, -1)) {};
	~RayTracer() {};

	glm::vec3 TraceRay(Ray ray)
	{
		// Gets shapes list from scene
		std::list<BaseShape*> shapes = mCurrentScene.GetShapes();

		// Initialises default closest hit and shape variables
		HitData closestHit{ false, glm::vec3(0, 0, 0) };
		BaseShape* closestShape;

		// Cycle through list
		for (BaseShape* currentShape : shapes)
		{
			// Check for collision
			HitData currentHitData = currentShape->GetHit(ray);

			// If collision detected
			if (currentHitData.mHit)
			{
				// Check if closest collision
				if (!closestHit.mHit || get_length_between_points(currentHitData.mFirstIntersection, ray.GetOrigin()) < get_length_between_points(closestHit.mFirstIntersection, ray.GetOrigin()))
				{
					// Update closest hit and shape variables
					closestHit = currentHitData;
					closestShape = currentShape;
				};
			};
		};

		// If collision detected
		if (closestHit.mHit)
		{
			// Gets colour modifier from closest shape
			float colourModifier = mCurrentScene.GetColourModifier(closestShape, closestHit.mFirstIntersection);

			// If collision, return colour
			return closestShape->GetColour() * colourModifier;
		};

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
	// Window and viewing variables
	glm::ivec2 mWindowSize;
	glm::ivec2 mWindowCentre;
	glm::ivec2 mViewingSize;

	// Used for working out ray origins and directions
	float mXViewMultiplier;
	float mYViewMultiplier;
	float mXViewOffset;
	float mYViewOffset;

public:
	Camera(glm::ivec2 windowSize, glm::ivec2 viewingSize)
	{
		// Getting window size, centre and viewing size
		mWindowSize = windowSize;
		mWindowCentre = windowSize / 2;
		mViewingSize = viewingSize;

		// Getting variables for calculating ray data
		mXViewMultiplier = (float)mViewingSize.x / (float)mWindowSize.x;
		mYViewMultiplier = (float)mViewingSize.y / (float)mWindowSize.y;
		mXViewOffset = (float)(mViewingSize.x - mWindowSize.x) / 2;
		mYViewOffset = (float)(mViewingSize.y - mWindowSize.y) / 2;
	};
	~Camera() {};

	Ray GetRay(glm::ivec2 pixelPosition)
	{
		// Getting start and end points for reference when creating the ray
		glm::vec3 source;
		glm::vec3 lead;

		// Getting coordinates for the ray's origin
		source.x = (float)pixelPosition.x;
		source.y = (float)pixelPosition.y;
		source.z = -1.f;

		// Getting coordinates for the ray's path
		lead.x = (float)(pixelPosition.x) * mXViewMultiplier - mXViewOffset;
		lead.y = (float)(pixelPosition.y) * mYViewMultiplier - mYViewOffset;
		lead.z = 20.f;

		// Creating ray
		Ray ray(source, glm::normalize(lead - source));

		return ray;
	};
};


// Outputs a vec3 to console (used for debugging)
void display_vec3(glm::vec3 vec)
{
	std::cout << vec.x << ", " << vec.y << ", " << vec.z << std::endl;
};


// Gets the area of a triangle
float area_of_triangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
};


// Checks if point is inside triangle
bool point_inside_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int px, int py)
{
	// Get area of ABC
	float A = area_of_triangle(x1, y1, x2, y2, x3, y3);

	// Get area o PBC
	float A1 = area_of_triangle(px, py, x2, y2, x3, y3);

	// Get area o PAC
	float A2 = area_of_triangle(x1, y1, px, py, x3, y3);

	// Get area o PAB
	float A3 = area_of_triangle(x1, y1, x2, y2, px, py);

	// Checks if sum of A1, A2 and A3 is same as A
	return (A == A1 + A2 + A3);
};


// Gets if 3D ray intersects 2D triangle
HitData get_ray_triangle_intersection(Ray ray, float z, glm::vec2 pointA, glm::vec2 pointB, glm::vec2 pointC)
{
	// Gets point at correct z coordinate
	glm::vec3 intersect_point = get_point_at_z(ray, z);

	// Test if point is inside triangle
	if (point_inside_triangle(pointA.x, pointA.y, pointB.x, pointB.y, pointC.x, pointC.y, intersect_point.x, intersect_point.y))
	{
		// Return collision detected
		return HitData{ true, intersect_point };
	};

	// Return no collision detected
	return HitData{ false, intersect_point };
};


// Gets if 3D ray intersects 2D rectangle
HitData get_ray_rectangle_intersection(Ray ray, glm::vec3 rect_pos, float rect_width, float rect_height)
{
	// Gets point at correct z coordinate
	glm::vec3 intersect_point = get_point_at_z(ray, rect_pos.z);

	// Gets rectangle boundaries
	float left_bd = rect_pos.x - (rect_width / 2);
	float right_bd = rect_pos.x + (rect_width / 2);
	float upper_bd = rect_pos.y - (rect_height / 2);
	float lower_bd = rect_pos.y + (rect_height / 2);

	// Checks if point is inside boundaries
	if (intersect_point.x >= left_bd && intersect_point.x <= right_bd && intersect_point.y >= upper_bd && intersect_point.y <= lower_bd)
	{
		// Returns collision detected
		return HitData{ true, intersect_point };
	};

	// Returns no collision detected
	return HitData{ false, intersect_point };
};


// Gets if 3D ray intersects 2D circle
HitData get_ray_circle_intersection(Ray ray, glm::vec3 circle_pos, float circle_radius)
{
	// Gets point at correct z coordinate
	HitData rect_hitdata = get_ray_rectangle_intersection(ray, circle_pos, circle_radius * 2, circle_radius * 2);

	// Checks if point is inside the circle
	if (rect_hitdata.mHit && get_length_between_points(rect_hitdata.mFirstIntersection, circle_pos) <= circle_radius)
	{
		// Returns collision detected
		return rect_hitdata;
	};

	// Returns no collision detected
	return HitData{ false, glm::vec3(0, 0, 0) };
};


// Returns 2D position at given z coordinate
glm::vec3 get_point_at_z(Ray ray, float z)
{
	// Gets ray values
	glm::vec3 origin = ray.GetOrigin();
	glm::vec3 direction = ray.GetDirection();

	// Gets z travel distance required
	float travel_distance = z - origin.z;
	// Gets vector multiplier needed to reach desired z coordinate
	float vector_multiplier = travel_distance / direction.z;

	// Gets vector at new position
	glm::vec3 pos_3d = origin + (direction * vector_multiplier);

	// Returns vector
	return pos_3d;
};


// Gets difference in direction
float get_direction_difference(glm::vec3 dir1, glm::vec3 dir2)
{
	// Normalises vectors
	glm::vec3 n_dir1 = glm::normalize(dir1);
	glm::vec3 n_dir2 = glm::normalize(dir2);

	// Gets difference between vectors
	float dif = glm::length(n_dir1 - n_dir2) / 2;

	// Returns difference
	return dif;
};


// Returns normal to the given sphere at given point
glm::vec3 get_normal_on_sphere(Sphere sphere, glm::vec3 queryPoint)
{
	// Get centre of sphere
	glm::vec3 sphereCentre = sphere.GetPos();
	// Calculate normal vector
	glm::vec3 normal = queryPoint - sphereCentre;

	// Return normal vector
	return glm::normalize(normal);
};


// Checks if the given point is inside the given sphere
bool check_inside_sphere(Sphere sphere, glm::vec3 queryPoint)
{
	// Gets centre of sphere
	glm::vec3 sphereCentre = sphere.GetPos();

	// Gets distance from point to centre
	int distanceToCentre = glm::length(sphereCentre - queryPoint);

	// Checks if distance is less than or equal to radius
	if (distanceToCentre <= sphere.GetRadius())
	{
		// Inside sphere
		return true;
	};

	// Not inside sphere
	return false;
};


// Checks if the given point is ahead of the given ray
bool check_ahead_ray(Ray ray, glm::vec3 queryPoint)
{
	float margin = glm::length(glm::normalize(ray.GetDirection()) - glm::normalize(queryPoint - ray.GetOrigin()));

	if (margin < 0.001)
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
	// Getting ray data
	glm::vec3 a = line.GetOrigin();
	glm::vec3 n = line.GetDirection();
	glm::vec3 P = queryPoint;
	
	// Working out closest point on ray to given point
	glm::vec3 closestPoint = a + (glm::dot((P - a), n)) * n;

	// Returns closest point vector
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
	// Get centre and radius of sphere
	glm::vec3 sphereCentre = sphere.GetPos();
	int sphereRadius = sphere.GetRadius();

	// Get ray data
	glm::vec3 a = ray.GetOrigin();
	glm::vec3 n = ray.GetDirection();
	glm::vec3 P = sphereCentre;

	// Checks if ray origin is inside sphere, if so, treats as an error and returns no intersection
	if (check_inside_sphere(sphere, a))
	{
		// Ray origin inside sphere
		return HitData{ false, glm::vec3(0,0,0) };
	};

	// Gets closest point to sphere centre
	glm::vec3 closestPoint = get_closest_point_on_line(ray, sphereCentre);
	// Gets length between closest point and sphere centre
	float d = glm::length(sphereCentre - closestPoint);
	int x = sqrt(pow(sphereRadius, 2) - pow(d, 2));

	// Checks if the closest point is ahead of the ray, if it's not, no intersection
	if (!check_ahead_ray(ray, closestPoint))
	{
		// Ray is heading backwards, ignores collision
		return HitData{ false, glm::vec3(0,0,0) };
	};

	// Checks if distance is less than or equal to sphere radius
	if (d <= sphereRadius)
	{
		// Valid collision detected
		// Gets point of intersection
		glm::vec3 firstIntersection = a + (glm::dot((P - a), n) - x) * n;

		// Returns collision data
		return HitData{ true, firstIntersection };
	};

	// No collision
	return HitData{ false, glm::vec3(0,0,0) };
};


float get_length_between_points(glm::vec3 point1, glm::vec3 point2)
{
	// Returns length between two given vectors
	return glm::length(point1 - point2);
};


// Gets position vector from user
glm::vec3 get_pos_from_user()
{
	int xPos, yPos, zPos;

	std::cout << "Enter position (3D): ";
	std::cin >> xPos >> yPos >> zPos;

	glm::vec3 pos(xPos, yPos, zPos);

	return pos;
};


// Gets 2D position vector from user
glm::vec2 get_2d_pos_from_user()
{
	int xPos, yPos;

	std::cout << "Enter position (2D): ";
	std::cin >> xPos >> yPos;

	glm::vec2 pos(xPos, yPos);

	return pos;
};


// Gets z coordinate from user
int get_z_from_user()
{
	int z;

	std::cout << "Enter z coordinate: ";
	std::cin >> z;

	return z;
};


// Gets colour vector from user
glm::vec3 get_colour_from_user()
{
	int r, g, b;

	std::cout << "Enter colour: ";
	std::cin >> r >> g >> b;

	glm::vec3 colour((float)r / 255, (float)g / 255, (float)b / 255);

	return colour;
};


// Gets radius from user
int get_radius_from_user()
{
	int radius;

	std::cout << "Enter radius: ";
	std::cin >> radius;

	return radius;
};


// Gets width value from user
int get_width_from_user()
{
	int width;

	std::cout << "Enter width: ";
	std::cin >> width;

	return width;
};


// Gets height value from user
int get_height_from_user()
{
	int height;

	std::cout << "Enter height: ";
	std::cin >> height;

	return height;
};


// Gets light direction vector from user
glm::vec3 get_light_direction_from_user()
{
	float x, y, z;

	std::cout << "Enter light direction vector: ";
	std::cin >> x >> y >> z;

	glm::vec3 direction(x, y, z);

	return direction;
};


int main( int argc, char *argv[] )
{
	// Variable for storing window dimensions
	glm::ivec2 windowSize( 640, 480 );
	glm::ivec2 viewingSize( 672, 504 );

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
	// Creates camera
	Camera camera(windowSize, viewingSize);

	// Gets light direction vector from user inputs
	glm::vec3 light_direction = get_light_direction_from_user();

	// Creates scene using given light direction vector
	Scene scene(light_direction);

	std::string option;

	// User input loop - allows the user to add objects into the scene
	bool ready{ false };
	while (!ready)
	{
		std::cout << "Shape menu:\n 1 - Rectangle\n 2 - Triangle\n 3 - Circle\n 4 - Sphere\n 5 - Done\nEnter option: ";
		std::cin >> option;

		if (option == "1")	// Creates rectangle
		{
			// Gets necessary data from user
			glm::vec3 pos = get_pos_from_user();
			int width = get_width_from_user();
			int height = get_height_from_user();
			glm::vec3 colour = get_colour_from_user();

			scene.AddRectangle(pos, width, height, colour);
		}
		else if (option == "2")	// Creates triangle
		{
			// Gets necessary data from user
			int z = get_z_from_user();
			glm::vec2 aPos = get_2d_pos_from_user();
			glm::vec2 bPos = get_2d_pos_from_user();
			glm::vec2 cPos = get_2d_pos_from_user();
			glm::vec3 colour = get_colour_from_user();

			scene.AddTriangle(z, aPos, bPos, cPos, colour);
		}	
		else if (option == "3")	// Creates circle
		{
			// Gets necessary data from user
			glm::vec3 pos = get_pos_from_user();
			int radius = get_radius_from_user();
			glm::vec3 colour = get_colour_from_user();

			scene.AddCircle(pos, radius, colour);
		}
		else if (option == "4")	// Creates sphere
		{
			// Gets necessary data from user
			glm::vec3 pos = get_pos_from_user();
			int radius = get_radius_from_user();
			glm::vec3 colour = get_colour_from_user();

			scene.AddSphere(pos, radius, colour);
		}
		else if (option == "5")	// Exits user input loop
		{
			ready = true;
		};
	};

	// Creates ray tracer and provides it with a scene
	RayTracer rayTracer;
	rayTracer.SetScene(scene);

	// Goes through each pixel on the screen
	for (int x = 0; x < windowSize.x; x++)
	{
		for (int y = 0; y < windowSize.y; y++)
		{
			// Gets pixel position vector
			glm::ivec2 pixelPosition(x, y);

			// Creates ray using pixel position vector
			Ray currentRay = camera.GetRay(pixelPosition);

			// Gets colour for that ray
			glm::vec3 pixelColour = rayTracer.TraceRay(currentRay);

			// Draws colour at pixel position
			MCG::DrawPixel(pixelPosition, pixelColour);
		};
	};

	// Displays drawing to screen and holds until user closes window
	// You must call this after all your drawing calls
	// Program will exit after this line
	return MCG::ShowAndHold();
}
