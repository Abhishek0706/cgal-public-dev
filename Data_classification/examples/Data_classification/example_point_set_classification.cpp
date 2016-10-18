#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Point_set_classification.h>
#include <CGAL/Data_classification/Planimetric_grid.h>
#include <CGAL/Data_classification/Attribute.h>
#include <CGAL/Data_classification/Attributes_eigen.h>
#include <CGAL/Data_classification/Attribute_distance_to_plane.h>
#include <CGAL/Data_classification/Attribute_vertical_dispersion.h>
#include <CGAL/Data_classification/Attribute_elevation.h>

#include <CGAL/IO/read_ply_points.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef Kernel::Iso_cuboid_3 Iso_cuboid_3;
typedef std::vector<Point>::iterator Iterator;
typedef CGAL::Identity_property_map<Point> Pmap;

typedef CGAL::Point_set_classification<Kernel, Iterator, Pmap> Classification;

typedef CGAL::Data_classification::Planimetric_grid<Kernel, Iterator, Pmap>      Planimetric_grid;
typedef CGAL::Data_classification::Neighborhood<Kernel, Iterator, Pmap>          Neighborhood;
typedef CGAL::Data_classification::Local_eigen_analysis<Kernel, Iterator, Pmap>  Local_eigen_analysis;

typedef CGAL::Data_classification::Type_handle                                           Type_handle;
typedef CGAL::Data_classification::Attribute_handle                                      Attribute_handle;

typedef CGAL::Data_classification::Attribute_distance_to_plane<Kernel, Iterator, Pmap>   Distance_to_plane;
typedef CGAL::Data_classification::Attribute_linearity<Kernel, Iterator, Pmap>           Linearity;
typedef CGAL::Data_classification::Attribute_omnivariance<Kernel, Iterator, Pmap>        Omnivariance;
typedef CGAL::Data_classification::Attribute_planarity<Kernel, Iterator, Pmap>           Planarity;
typedef CGAL::Data_classification::Attribute_surface_variation<Kernel, Iterator, Pmap>   Surface_variation;
typedef CGAL::Data_classification::Attribute_elevation<Kernel, Iterator, Pmap>           Elevation;
typedef CGAL::Data_classification::Attribute_vertical_dispersion<Kernel, Iterator, Pmap> Dispersion;



int main (int argc, char** argv)
{
  std::string filename (argc > 1 ? argv[1] : "data/b9.ply");
  std::ifstream in (filename.c_str());
  std::vector<Point> pts;

  std::cerr << "Reading input" << std::endl;
  if (!in
      || !(CGAL::read_ply_points (in, std::back_inserter (pts))))
    {
      std::cerr << "Error: cannot read " << filename << std::endl;
      return EXIT_FAILURE;
    }

  double grid_resolution = 0.34;
  double radius_neighbors = 1.7;
  double radius_dtm = 15.0;

  std::cerr << "Computing useful structures" << std::endl;

  Iso_cuboid_3 bbox = CGAL::bounding_box (pts.begin(), pts.end());
  Planimetric_grid grid (pts.begin(), pts.end(), Pmap(), bbox, grid_resolution);
  Neighborhood neighborhood (pts.begin(), pts.end(), Pmap());
  double garbage;
  Local_eigen_analysis eigen (pts.begin(), pts.end(), Pmap(), neighborhood, 6, garbage);
  
  std::cerr << "Computing attributes" << std::endl;
  Attribute_handle d2p (new Distance_to_plane (pts.begin(), pts.end(), Pmap(), eigen));
  Attribute_handle lin (new Linearity (pts.begin(), pts.end(), eigen));
  Attribute_handle omni (new Omnivariance (pts.begin(), pts.end(), eigen));
  Attribute_handle plan (new Planarity (pts.begin(), pts.end(), eigen));
  Attribute_handle surf (new Surface_variation (pts.begin(), pts.end(), eigen));
  Attribute_handle disp (new Dispersion (pts.begin(), pts.end(), Pmap(), grid,
                                         grid_resolution,
                                         radius_neighbors));
  Attribute_handle elev (new Elevation (pts.begin(), pts.end(), Pmap(), bbox, grid,
                                        grid_resolution,
                                        radius_neighbors,
                                        radius_dtm));
  
  std::cerr << "Setting weights" << std::endl;
  d2p->weight  = 6.75e-2;
  lin->weight  = 1.19;
  omni->weight = 1.34e-1;
  plan->weight = 7.32e-1;
  surf->weight = 1.36e-1;
  disp->weight = 5.45e-1;
  elev->weight = 1.47e1;

  // Add attributes to PSC
  Classification psc (pts.begin (), pts.end(), Pmap());
  psc.add_attribute (d2p);
  psc.add_attribute (lin);
  psc.add_attribute (omni);
  psc.add_attribute (plan);
  psc.add_attribute (surf);
  psc.add_attribute (disp);
  psc.add_attribute (elev);

  std::cerr << "Setting up classification types" << std::endl;
  
  // Create classification type and define how attributes affect them
  Type_handle ground = psc.add_classification_type ("ground");
  ground->set_attribute_effect (d2p,  CGAL::Data_classification::Type::NEUTRAL_ATT);
  ground->set_attribute_effect (lin,  CGAL::Data_classification::Type::PENALIZED_ATT);
  ground->set_attribute_effect (omni, CGAL::Data_classification::Type::NEUTRAL_ATT);
  ground->set_attribute_effect (plan, CGAL::Data_classification::Type::FAVORED_ATT);
  ground->set_attribute_effect (surf, CGAL::Data_classification::Type::PENALIZED_ATT);
  ground->set_attribute_effect (disp, CGAL::Data_classification::Type::NEUTRAL_ATT);
  ground->set_attribute_effect (elev, CGAL::Data_classification::Type::PENALIZED_ATT);

  Type_handle vege = psc.add_classification_type ("vegetation");
  vege->set_attribute_effect (d2p,  CGAL::Data_classification::Type::FAVORED_ATT);
  vege->set_attribute_effect (lin,  CGAL::Data_classification::Type::NEUTRAL_ATT);
  vege->set_attribute_effect (omni, CGAL::Data_classification::Type::FAVORED_ATT);
  vege->set_attribute_effect (plan, CGAL::Data_classification::Type::NEUTRAL_ATT);
  vege->set_attribute_effect (surf, CGAL::Data_classification::Type::NEUTRAL_ATT);
  vege->set_attribute_effect (disp, CGAL::Data_classification::Type::FAVORED_ATT);
  vege->set_attribute_effect (elev, CGAL::Data_classification::Type::NEUTRAL_ATT);
  
  Type_handle roof = psc.add_classification_type ("roof");
  roof->set_attribute_effect (d2p,  CGAL::Data_classification::Type::NEUTRAL_ATT);
  roof->set_attribute_effect (lin,  CGAL::Data_classification::Type::PENALIZED_ATT);
  roof->set_attribute_effect (omni, CGAL::Data_classification::Type::FAVORED_ATT);
  roof->set_attribute_effect (plan, CGAL::Data_classification::Type::FAVORED_ATT);
  roof->set_attribute_effect (surf, CGAL::Data_classification::Type::PENALIZED_ATT);
  roof->set_attribute_effect (disp, CGAL::Data_classification::Type::NEUTRAL_ATT);
  roof->set_attribute_effect (elev, CGAL::Data_classification::Type::FAVORED_ATT);

  // Run classification
  psc.run_with_graphcut (neighborhood, 0.2);
  //psc.run();
  
  // Save the output in a colored PLY format

  std::ofstream f ("classification.ply");
  f << "ply" << std::endl
    << "format ascii 1.0" << std::endl
    << "element vertex " << pts.size() << std::endl
    << "property float x" << std::endl
    << "property float y" << std::endl
    << "property float z" << std::endl
    << "property uchar red" << std::endl
    << "property uchar green" << std::endl
    << "property uchar blue" << std::endl
    << "end_header" << std::endl;
  
  for (std::size_t i = 0; i < pts.size(); ++ i)
    {
      f << pts[i] << " ";
      
      Type_handle type = psc.classification_type_of (i);
      if (type == ground)
        f << "245 180 0" << std::endl;
      else if (type == vege)
        f << "0 255 27" << std::endl;
      else if (type == roof)
        f << "255 0 170" << std::endl;
      else
        {
          f << "0 0 0" << std::endl;
          std::cerr << "Error: unknown classification type" << std::endl;
        }
    }
  
  std::cerr << "All done" << std::endl;
  return EXIT_SUCCESS;
}
