/*!
\ingroup PkgShapeDetectionRGConcepts
\cgalConcept

A concept that describes the set of methods used to access connected items in a 
given set by the `CGAL::Shape_detection::Region_growing` approach.

\cgalHasModel 
`CGAL::Shape_detection::Points_fuzzy_sphere_connectivity`, 
`CGAL::Shape_detection::Points_k_nearest_neighbors_connectivity`, 
`CGAL::Shape_detection::Polygon_mesh_adjacent_faces_connectivity`
*/

class RegionGrowingConnectivity {

public:

  /*!  
    Returns indices of all items, which are connected to an item with the index `query_index`.
    The indices are returned via an output iterator.

    This function is called each time when a new query item is chosen.

    \tparam OutputIterator An output iterator that accepts `std::size_t` values.
  */
  void get_neighbors(
    std::size_t query_index, 
    OutputIterator& neighbors) {
        
  }
};
