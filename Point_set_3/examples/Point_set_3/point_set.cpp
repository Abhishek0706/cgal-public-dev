#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>

#include <fstream>
#include <limits>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;

typedef CGAL::Point_set_3<Kernel> Point_set;

void print_point_set (const Point_set& point_set)
{
  std::cerr << "Content of point set:" << std::endl;
  for (Point_set::const_iterator it = point_set.begin();
       it != point_set.end(); ++ it)
    std::cerr << "* Point " << point_set.point(it) // or point_set[it]
              << " with normal " << point_set.normal(it)
              << std::endl;
}

int main (int, char**)
{
  Point_set point_set;

  // Add points
  point_set.push_back (Point (0., 0., 0.));
  point_set.push_back (Point (0., 0., 1.));
  point_set.push_back (Point (0., 1., 0.));

  point_set.add_normal_property();

  print_point_set(point_set); // Normals have default values

  // Change normal values
  point_set.normal(0) = Vector (1., 0., 0.);
  point_set.normal(1) = Vector (0., 1., 0.);
  point_set.normal(2) = Vector (0., 0., 1.);

  // Add point + normal
  point_set.push_back (Point (1., 2., 3.), Vector (4., 5., 6.));

  print_point_set(point_set); 

  // Add new item
  Point_set::iterator new_item = point_set.add_item();

  print_point_set(point_set); // New item has default values

  point_set.remove (point_set.begin());

  print_point_set(point_set); // New item has default values
  
  // Display information
  std::cerr << point_set.info();

  point_set.collect_garbage();
  
  // Display information (garbage should be gone)
  std::cerr << point_set.info();

  return 0;
}
