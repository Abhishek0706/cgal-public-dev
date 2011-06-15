#include <CGAL/Combinatorial_map.h>
#include <CGAL/Combinatorial_map_constructors.h>
#include <CGAL/Combinatorial_map_operations.h>
#include <iostream>
#include <cstdlib>

typedef CGAL::Combinatorial_map<3> CMap_3;
typedef CMap_3::Dart_handle        Dart_handle;

int main()
{
  CMap_3 cm;

  // Create one .
  Dart_handle d1 = CGAL::make_combinatorial_hexahedron(cm);

  // Add two edges along two opposite facets.
  CGAL::insert_cell_1_in_cell_2(cm,d1->beta(1),d1->beta(0));
  CGAL_assertion( cm.is_valid() );

  Dart_handle d2=d1->beta(2)->beta(1)->beta(1)->beta(2);
  CGAL::insert_cell_1_in_cell_2(cm,d2,d2->beta(1)->beta(1));
  CGAL_assertion( cm.is_valid() );

  // Insert a facet along these two new edges plus two initial edges of the cube.
  std::vector<Dart_handle> path;
  path.push_back(d1->beta(1));
  path.push_back(d1->beta(0)->beta(2)->beta(1));
  path.push_back(d2->beta(0));
  path.push_back(d2->beta(2)->beta(1));
  
  Dart_handle d3=CGAL::insert_cell_2_in_cell_3(cm,path.begin(),path.end());
  CGAL_assertion( cm.is_valid() );
  
  // Display the m characteristics.
  cm.display_characteristics(std::cout) << ", valid=" << 
    cm.is_valid() << std::endl;

  // We use the removal operations to get back to the initial cube.
  CGAL::remove_cell<CMap_3,2>(cm,d3);
  CGAL_assertion( cm.is_valid() );

  CGAL::remove_cell<CMap_3,1>(cm,d1->beta(1));
  CGAL_assertion( cm.is_valid() );

  CGAL::remove_cell<CMap_3,1>(cm,d2->beta(0));
  CGAL_assertion( cm.is_valid() );
  
  // Display the m characteristics.
  cm.display_characteristics(std::cout) << ", valid=" 
					<< cm.is_valid() << std::endl;

  return EXIT_SUCCESS;
}

