// TODO: Copyright info
// TODO: Requires both OpenGR and PointMatcher wrappers.

#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/property_map.h>
#include <CGAL/Aff_transformation_3.h>

#include <CGAL/pointmatcher/compute_registration_transformation.h>

#include <CGAL/OpenGR/compute_registration_transformation.h>

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

typedef CGAL::Simple_cartesian<float> K;
typedef K::Point_3 Point_3;
typedef K::Vector_3 Vector_3;
typedef std::pair<Point_3, Vector_3> Pwn;
typedef CGAL::First_of_pair_property_map<Pwn> Point_map;
typedef CGAL::Second_of_pair_property_map<Pwn> Normal_map;

namespace params = CGAL::parameters;

int main(int argc, const char** argv)
{
  const char* fname1 = (argc>1)?argv[1]:"data/hippo1.xyz";
  const char* fname2 = (argc>2)?argv[2]:"data/hippo2.xyz";

  std::vector<Pwn> pwns1, pwns2;
  std::ifstream input(fname1);
  if (!input ||
      !CGAL::read_xyz_points(input, std::back_inserter(pwns1),
            CGAL::parameters::point_map (CGAL::First_of_pair_property_map<Pwn>()).
            normal_map (Normal_map())))
  {
    std::cerr << "Error: cannot read file " << fname1 << std::endl;
    return EXIT_FAILURE;
  }
  input.close();

  input.open(fname2);
  if (!input ||
      !CGAL::read_xyz_points(input, std::back_inserter(pwns2),
            CGAL::parameters::point_map (Point_map()).
            normal_map (Normal_map())))
  {
    std::cerr << "Error: cannot read file " << fname2 << std::endl;
    return EXIT_FAILURE;
  }
  input.close();
  
  std::cout << "Computing registration transformation using OpenGR Super4PCS.." << std::endl;
  // First, compute registration transformation using OpenGR Super4PCS
  K::Aff_transformation_3 res =
    std::get<0>( // get first of pair, which is the transformation
      CGAL::OpenGR::compute_registration_transformation
        (pwns1, pwns2,
         params::point_map(Point_map()).normal_map(Normal_map()),
         params::point_map(Point_map()).normal_map(Normal_map()))
    );

  std::cout << "Computing registration transformation using PointMatcher ICP, "
            << "taking transformation computed by OpenGR Super4PCS as initial transformation.." << std::endl;
  // Then, compute registration transformation using PointMatcher ICP, taking transformation computed
  // by OpenGR as initial transformation
  res =
    CGAL::pointmatcher::compute_registration_transformation
      (pwns1, pwns2, 
       params::point_map(Point_map()).normal_map(Normal_map()),
       params::point_map(Point_map()).normal_map(Normal_map())
       .transformation(res)
      );

  std::ofstream out("pwns2_aligned.xyz");
  if (!out ||
      !CGAL::write_xyz_points(
        out, pwns2,
        CGAL::parameters::point_map(Point_map()).
        normal_map(Normal_map())))
  {
    return EXIT_FAILURE;
  }

  std::cout << "Transformed version of " << fname2
            << " written to pwn2_aligned.xyz.\n";

  return EXIT_SUCCESS;
}