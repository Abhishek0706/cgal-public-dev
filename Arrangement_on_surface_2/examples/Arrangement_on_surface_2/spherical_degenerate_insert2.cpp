//! \file examples/Arrangement_on_surface_2/aggregated_insertion.cpp
// Using the global aggregated insertion functions.

#include <list>

#include <CGAL/basic.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Arrangement_on_surface_2.h>
#include <CGAL/Arr_geodesic_arc_on_sphere_traits_2.h>
#include <CGAL/Arr_spherical_topology_traits_2.h>

typedef CGAL::Exact_rational                               Number_type;
typedef CGAL::Cartesian<Number_type>                       Kernel;
typedef CGAL::Arr_geodesic_arc_on_sphere_traits_2<Kernel>  Geom_traits_2;
typedef Geom_traits_2::Point_2                             Point_2;
typedef Geom_traits_2::X_monotone_curve_2                  X_monotone_curve_2;
typedef CGAL::Arr_spherical_topology_traits_2<Geom_traits_2> Topol_traits_2;
typedef CGAL::Arrangement_on_surface_2<Geom_traits_2, Topol_traits_2>
                                                           Arrangement_2;
typedef Arrangement_2::Vertex_handle                       Vertex_handle;

int main()
{
  Arrangement_2 arr;
  Point_2 sp(0, 0, -1);
  Point_2 np(0, 0, 1);

  Point_2 p1(-1,  0, -1);
  Point_2 p2(-1,  0,  1);
  X_monotone_curve_2 xcv_sp_p2(sp, p2);
  X_monotone_curve_2 xcv_np_p1(np, p1);
  insert(arr, xcv_sp_p2);
  insert(arr, xcv_np_p1);

  Point_2 q1(-1,  1, -1);
  Point_2 q2(-1,  1,  1);
  X_monotone_curve_2 xcv_sp_q2(sp, q2);
  X_monotone_curve_2 xcv_np_q1(np, q1);
  insert(arr, xcv_sp_q2);
  insert(arr, xcv_np_q1);

  // Print the size of the arrangement.
  std::cout << "The arrangement size:" << std::endl
            << "   V = " << arr.number_of_vertices()
            << ",  E = " << arr.number_of_edges()
            << ",  F = " << arr.number_of_faces() << std::endl;

  std::cout << "arr: " << arr << std::endl;

  return 0;
}
