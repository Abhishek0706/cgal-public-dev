// TODO: Copyright info

#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/property_map.h>

#include <CGAL/pointmatcher/compute_registration_transformation.h>
#include <CGAL/pointmatcher/register_point_sets.h>

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

  // TODO: Another example with default settings, refering to default settings in doc
  
  //
  // Prepare ICP config
  //
  using CGAL::pointmatcher::ICP_config;

  // TODO: Refer to https://github.com/ethz-asl/libpointmatcher/blob/master/doc/Configuration.md while doc

  // Prepare point set 1 filters (PM::ReadingDataPointsFilters)
  std::vector<ICP_config> point_set_1_filters;
  point_set_1_filters.push_back( ICP_config { .name="MinDistDataPointsFilter"       , .params={ {"minDist", "0.5" }}  } );
  point_set_1_filters.push_back( ICP_config { .name="RandomSamplingDataPointsFilter", .params={ {"prob"   , "0.05"}}  } );

  // Prepare point set 2 filters (PM::ReferenceDataPointsFilters)
  std::vector<ICP_config> point_set_2_filters;
  point_set_2_filters.push_back( ICP_config { .name="MinDistDataPointsFilter"       , .params={ {"minDist", "0.5" }}  } );
  point_set_2_filters.push_back( ICP_config { .name="RandomSamplingDataPointsFilter", .params={ {"prob"   , "0.05"}}  } );

	// Prepare matcher function
  ICP_config matcher { .name="KDTreeMatcher", .params={ {"knn", "1"}, {"epsilon", "3.16"} } };

  // Prepare outlier filters
  std::vector<ICP_config> outlier_filters;
  outlier_filters.push_back( ICP_config { .name="TrimmedDistOutlierFilter", .params={ {"ratio", "0.75" }}  } );

  // Prepare error minimizer
  ICP_config error_minimizer { .name = "PointToPointErrorMinimizer"};

  // Prepare transformation checker
  std::vector<ICP_config> transformation_checkers;
  transformation_checkers.push_back( ICP_config { .name="CounterTransformationChecker", .params={ {"maxIterationCount", "150" }}  } );
  transformation_checkers.push_back( ICP_config { .name="DifferentialTransformationChecker", .params={ {"minDiffRotErr"  , "0.001" },
                                                                                                       {"minDiffTransErr", "0.01"  },
                                                                                                       {"smoothLength"   , "4"     } }  
                                                } );
  // Prepare inspector
  ICP_config inspector { .name="NullInspector" };

  // Prepare logger
  ICP_config logger { .name= "FileLogger" };

  // EITHER call the ICP registration method pointmatcher to get the transformation to apply to pwns2 TODO: or pwns1?
  K::Aff_transformation_3 res =
  CGAL::pointmatcher::compute_registration_transformation
    (pwns1, pwns2, 
     params::point_map(Point_map()).normal_map(Normal_map())
     .point_set_filters(point_set_1_filters)
     .matcher(matcher)
     .outlier_filters(outlier_filters)
     .error_minimizer(error_minimizer)
     .transformation_checkers(transformation_checkers)
     .inspector(inspector)
     .logger(logger),
     params::point_map(Point_map()).normal_map(Normal_map())
     .point_set_filters(point_set_2_filters));

  // OR call the ICP registration method from pointmatcher and apply the transformation to pwn1
  // TODO: Check the latest convention on ref point cloud. Currently, point_set_2 is the ref, as in pointmatcher
  //       Therefore, the transformation is applied on point_set_1
  CGAL::pointmatcher::register_point_sets
    (pwns1, pwns2, 
     params::point_map(Point_map()).normal_map(Normal_map())
     .point_set_filters(point_set_1_filters)
     .matcher(matcher)
     .outlier_filters(outlier_filters)
     .error_minimizer(error_minimizer)
     .transformation_checkers(transformation_checkers)
     .inspector(inspector)
     .logger(logger),
     params::point_map(Point_map()).normal_map(Normal_map())
     .point_set_filters(point_set_2_filters));

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