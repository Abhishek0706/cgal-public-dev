#include <iostream>
#include <limits>
#include <cmath>
#include <cassert>
#include "test_triangulation_2_types.h"
#include <CGAL/properties/triangulation_2.h>

using namespace CGAL::Properties::Triangulation_2;

bool approx_eq(double a, double b)
{
  static double epsilon = 1e-3;
  std::cout << a << " " << b << std::endl;
  return std::fabs(a-b) <= epsilon;
}

using namespace std;

int main()
{
  double infinity = std::numeric_limits<double>::infinity(); 

  // Triangulation for testing.
  Delaunay dt;
 
  Vertex_handle v_1 = dt.insert(Point(  0.0,  0.0  ));
  dt.insert(Point(  1.0,  0.0  ));
  dt.insert(Point(  0.0,  1.0  ));
  dt.insert(Point(  0.0, -2.0  ));

  Face_handle f_1 = dt.locate(Point(0.2,  0.2));
  Face_handle f_2 = dt.locate(Point(0.2, -0.2));

  // An infinite face.
  Face_handle f_3 = dt.locate(Point(3.0,  3.0));

  // Check helper functions compile as expected.
  make_area(dt);
  make_area(dt, CGAL::Finite_test_tag());
  make_area(dt, CGAL::No_finite_test_tag());

  make_aspect_ratio(dt);
  make_aspect_ratio(dt, CGAL::Finite_test_tag());
  make_aspect_ratio(dt, CGAL::No_finite_test_tag());

  make_circumradius(dt);
  make_circumradius(dt, CGAL::Finite_test_tag());
  make_circumradius(dt, CGAL::No_finite_test_tag());

  make_min_angle(dt);
  make_min_angle(dt, CGAL::Finite_test_tag());
  make_min_angle(dt, CGAL::No_finite_test_tag());

  make_max_angle(dt);
  make_max_angle(dt, CGAL::Finite_test_tag());
  make_max_angle(dt, CGAL::No_finite_test_tag());

  make_min_edge_length(dt);
  make_min_edge_length(dt, CGAL::Finite_test_tag());
  make_min_edge_length(dt, CGAL::No_finite_test_tag());

  make_max_edge_length(dt);
  make_max_edge_length(dt, CGAL::Finite_test_tag());
  make_max_edge_length(dt, CGAL::No_finite_test_tag());


  //-- Area ------------------------------------------------------------------//

  Area<Delaunay, CGAL::No_finite_test_tag> 
    area_a;   
  Area<Delaunay, CGAL::Finite_test_tag>    
    area_b(dt);
  Area<Delaunay>                           
    area_c(dt);

  assert(  approx_eq(area_a(f_1), 0.5)  );
  assert(  approx_eq(area_b(f_2), 1.0)  );
  assert(  approx_eq(area_c(f_2), 1.0)  );
  assert(  area_b(f_3) == infinity  );
  assert(  area_c(f_3) == infinity  );


  //-- Aspect_ratio ----------------------------------------------------------//

  Aspect_ratio<Delaunay, CGAL::No_finite_test_tag> 
    aspect_ratio_a;   
  Aspect_ratio<Delaunay, CGAL::Finite_test_tag>    
    aspect_ratio_b(dt);
  Aspect_ratio<Delaunay>                           
    aspect_ratio_c(dt);

  assert(  approx_eq(aspect_ratio_a(f_1), 1.41421)  );
  assert(  approx_eq(aspect_ratio_a(f_2), 2.23606)  );
  assert(  approx_eq(aspect_ratio_b(f_2), 2.23606)  );
  assert(  approx_eq(aspect_ratio_c(f_2), 2.23606)  );
  assert(  aspect_ratio_b(f_3) == infinity  );
  assert(  aspect_ratio_c(f_3) == infinity  );


  //-- Circumradius ----------------------------------------------------------//
  
  Circumradius<Delaunay, CGAL::No_finite_test_tag> 
    circumradius_a;   
  Circumradius<Delaunay, CGAL::Finite_test_tag>    
    circumradius_b(dt);
  Circumradius<Delaunay>                           
    circumradius_c(dt);

  assert(  approx_eq(circumradius_a(f_1), 0.70710)  );
  assert(  approx_eq(circumradius_b(f_2), 1.11803)  );
  assert(  approx_eq(circumradius_c(f_2), 1.11803)  );
  assert(  circumradius_b(f_3) == infinity  );
  assert(  circumradius_c(f_3) == infinity  );


  //-- Angle -----------------------------------------------------------------//
  
  Angle<Delaunay, CGAL::No_finite_test_tag> 
    angle_a;   
  Angle<Delaunay, CGAL::Finite_test_tag>    
    angle_b(dt);
  Angle<Delaunay>                           
    angle_c(dt);

  assert(  approx_eq(angle_a(f_1,  f_1->index(v_1)         ), CGAL_PI/2)  );
  assert(  approx_eq(angle_a(f_1, (f_1->index(v_1) + 1) % 3), CGAL_PI/4)  );
  assert(  approx_eq(angle_a(f_2, (f_2->index(v_1) + 1) % 3), 0.4636476)  );

  //-- Min_angle --------------------------------------------------------------//
  
  Min_angle<Delaunay, CGAL::No_finite_test_tag> 
    min_angle_a;   
  Min_angle<Delaunay, CGAL::Finite_test_tag>    
    min_angle_b(dt);
  Min_angle<Delaunay>                           
    min_angle_c(dt);

  assert(  approx_eq(min_angle_a(f_1), CGAL_PI/4)  );
  assert(  approx_eq(min_angle_a(f_2), 0.4636476)  );

  assert(  approx_eq(min_angle_b(f_3), 0.0)  );
  assert(  approx_eq(min_angle_c(f_3), 0.0)  );


  //-- Max_angle -------------------------------------------------------------//
  
  Max_angle<Delaunay, CGAL::No_finite_test_tag> 
    max_angle_a;   
  Max_angle<Delaunay, CGAL::Finite_test_tag>    
    max_angle_b(dt);
  Max_angle<Delaunay>                           
    max_angle_c(dt);

  assert(  approx_eq(max_angle_a(f_1), CGAL_PI/2)  );
  assert(  approx_eq(max_angle_a(f_2), CGAL_PI/2)  );
  assert(  approx_eq(max_angle_b(f_3), CGAL_PI)  );
  assert(  approx_eq(max_angle_c(f_3), CGAL_PI)  );


  //-- Min_edge_length -------------------------------------------------------//
  
  Min_edge_length<Delaunay, CGAL::No_finite_test_tag> 
    min_edge_length_a;   
  Min_edge_length<Delaunay, CGAL::Finite_test_tag>    
    min_edge_length_b(dt);
  Min_edge_length<Delaunay>                           
    min_edge_length_c(dt);

  assert(  approx_eq(min_edge_length_a(f_1), 1.0)  );
  assert(  approx_eq(min_edge_length_b(f_1), 1.0)  );
  assert(  approx_eq(min_edge_length_c(f_1), 1.0)  );
  assert(  approx_eq(min_edge_length_a(f_2), 1.0)  );
  assert(  approx_eq(min_edge_length_b(f_2), 1.0)  );
  assert(  approx_eq(min_edge_length_c(f_2), 1.0)  );
  assert(  approx_eq(min_edge_length_c(f_3), 1.41421)  );
  assert(  approx_eq(min_edge_length_b(f_3), 1.41421)  );


  //-- Max_edge_length -------------------------------------------------------//

  Max_edge_length<Delaunay, CGAL::No_finite_test_tag> 
    max_edge_length_a;   
  Max_edge_length<Delaunay, CGAL::Finite_test_tag>    
    max_edge_length_b(dt);
  Max_edge_length<Delaunay>                           
    max_edge_length_c(dt);

  assert(  approx_eq(max_edge_length_a(f_1), 1.41421)  );
  assert(  approx_eq(max_edge_length_b(f_1), 1.41421)  );
  assert(  approx_eq(max_edge_length_c(f_1), 1.41421)  );
  assert(  approx_eq(max_edge_length_a(f_2), 2.23606)  );
  assert(  approx_eq(max_edge_length_b(f_2), 2.23606)  );
  assert(  approx_eq(max_edge_length_c(f_2), 2.23606)  );

  assert(  max_edge_length_b(f_3) == infinity  );
  assert(  max_edge_length_c(f_3) == infinity  );

}
