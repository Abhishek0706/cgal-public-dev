
namespace CGAL {
  /*!
    \ingroup PkgSurfaceMeshTopologyClasses

    The class `Combinatorial_map_2_incremental_builder` is a tool enabling to build incrementally a 2D combinatorial map.

    \tparam CMap a model of `CombinatorialMap`
  */
  template<typename CMap>
  class Combinatorial_map_2_incremental_builder
  {
  public:
    /// Dart_handle type.
    typedef typename CMap::Dart_handle Dart_handle;
    
    /*! creates a `Combinatorial_map_2_incremental_builder` object, to create amap.
     */
    Combinatorial_map_2_incremental_builder(CMap& amap);
    
    /// starts a new surface
    void begin_surface();
      
    /// finishs the current surface. Returns one dart of the created surface.
    /// @pre A surface is under creation.
    Dart_handle end_surface();

    /// starts a new facet.
    void begin_facet();
    
    /// finishs the current facet. Returns the first dart of this facet.
    /// @pre A facet is under creation.
    Dart_handle end_facet();

    /// adds one edge to the current facet, given by its label `s` (any string, using minus sign for orientation)
    /// @pre A facet is under creation.
    void add_edge_to_facet(const std::string& s);
    
    /// adds the given edges to the current facet.
    /// `s` is a sequence of labels, add all the corresponding edges into the current facet.
    /// @pre A facet is under creation.
    void add_edges_to_facet(const std::string& s);
    
    /// adds directly one facet giving the sequence of labels `s` of all its edges.
    /// @pre A surface is under creation.
    void add_facet(const std::string& s);
      
   /// starts a path on the surface.
   void begin_path();
    
    /// finishs the current path. Returns the path created.
    /// @pre A path is under creation.
    CGAL::Path_on_surface<CMap> end_path();

    /// adds edge labeled `e` at the end of the current path.
    /// @pre A path is under creation.
    void add_edge_to_path(const std::string& e);
    
    /// create a path directly from a sequence of edge labels `s`. Returns the path created.
    CGAL::Path_on_surface<CMap> create_path(const std::string& s);
  };
}