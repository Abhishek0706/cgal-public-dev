// Copyright (c) 2007  INRIA (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
//
// Author(s)     : Laurent Saboret, Pierre Alliez, Tong Zhao


#ifndef CGAL_IMPLICIT_FCT_DELAUNAY_TRIANGULATION_H
#define CGAL_IMPLICIT_FCT_DELAUNAY_TRIANGULATION_H

#include <CGAL/license/Implicit_surface_reconstruction_3.h>

#include <CGAL/disable_warnings.h>

#include <CGAL/Point_with_normal_3.h>
#include <CGAL/Lightweight_vector_3.h>
#include <CGAL/property_map.h>
#include <CGAL/surface_reconstruction_points_assertions.h>

#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include <CGAL/algorithm.h>
#include <CGAL/bounding_box.h>
#include <boost/random/random_number_generator.hpp>
#include <boost/random/linear_congruential.hpp>


#include <vector>
#include <iterator>

namespace CGAL {

/// \internal
/// The Reconstruction_vertex_base_3 class is the default
/// vertex class of the Reconstruction_triangulation_3 class.
///
/// It provides the interface requested by the Implicit_reconstruction_function class:
/// - Each vertex stores a normal vector.
/// - A vertex is either an input point or a Steiner point added by Delaunay refinement.
/// - In order to solve a linear system over the triangulation, a vertex may be constrained
///   or not (i.e. may contribute to the right or left member of the linear system),
///   and has a unique index.
///
/// @param Gt   Geometric traits class / Point_3 is a typedef to Point_with_normal_3.
/// @param Cb   Vertex base class, model of TriangulationVertexBase_3.

template < typename Gt,
           typename Vb = Triangulation_vertex_base_3<Gt> >
class Reconstruction_vertex_base_3 : public Vb
{
// Public types
public:

  /// Geometric traits class / Point_3 is a typedef to Point_with_normal_3.
  typedef Gt Geom_traits;

  // Repeat Triangulation_vertex_base_3 public types
  /// \cond SKIP_IN_MANUAL
  typedef typename Vb::Cell_handle Cell_handle;
  template < typename TDS2 >
  struct Rebind_TDS {
    typedef typename Vb::template Rebind_TDS<TDS2>::Other                       Vb2;
    typedef Reconstruction_vertex_base_3<Geom_traits, Vb2> Other;
  };
  /// \endcond

  // Geometric types
  typedef typename Geom_traits::FT FT;
  typedef typename Geom_traits::Vector_3 Vector;           ///< typedef to Vector_3
  typedef typename Geom_traits::Point_3 Point;             ///< typedef to Point_with_normal_3
  typedef typename Geom_traits::Point_3 Point_with_normal; ///< typedef to Point_with_normal_3

// data members
private:

  // TODO: reduce memory footprint
  FT m_f; // value of the implicit function // float precise enough?
  bool m_constrained; // is vertex constrained? // combine constrained and type
  unsigned char m_type; // INPUT or STEINER
  unsigned int m_index; // index in matrix (to be stored outside)
  FT m_lf;
  FT m_v;
  FT m_af;

// Public methods
public:

  Reconstruction_vertex_base_3()
    : Vb(), m_f(FT(0.0)), m_type(0), m_index(0)
  {}

  Reconstruction_vertex_base_3(const Point_with_normal& p)
    : Vb(p), m_f(FT(0.0)), m_type(0), m_index(0)
  {}

  Reconstruction_vertex_base_3(const Point_with_normal& p, Cell_handle c)
    : Vb(p,c), m_f(FT(0.0)), m_type(0), m_index(0)
  {}

  Reconstruction_vertex_base_3(Cell_handle c)
    : Vb(c), m_f(FT(0.0)), m_type(0), m_index(0)
  {}


  /// Gets/sets the value of the implicit function.
  /// Default value is 0.0.
  FT  f() const { return m_f; }
  FT& f()       { return m_f; }

  FT  lf() const { return m_lf; }
  FT& lf()       { return m_lf; }

  FT  v() const { return m_v; }
  FT& v()       { return m_v; }

  FT  af() const { return m_af; }
  FT& af()       { return m_af; }

  /// Gets/sets the type = INPUT or STEINER.
  unsigned char  type() const { return m_type; }
  unsigned char& type()       { return m_type; }

  /// Gets/sets the index in matrix.
  unsigned int  index() const { return m_index; }
  unsigned int& index()       { return m_index; }

  /// Gets/sets normal vector.
  /// Default value is null vector.
  const Vector& normal() const { return this->point().normal(); }
  Vector&       normal()       { return this->point().normal(); }

// Private methods
private:

    /// Copy constructor and operator =() are not implemented.
    Reconstruction_vertex_base_3(const Reconstruction_vertex_base_3& toCopy);
    Reconstruction_vertex_base_3& operator =(const Reconstruction_vertex_base_3& toCopy);

}; // end of Reconstruction_vertex_base_3


/// \internal
/// Helper class:
/// Reconstruction_triangulation_default_geom_traits_3
/// changes in a geometric traits class the Point_3 type to
/// Point_with_normal_3<BaseGt>.
///
/// @param BaseGt   Geometric traits class.
template <class BaseGt>
struct Reconstruction_triangulation_default_geom_traits_3 : public BaseGt
{
  typedef Point_with_normal_3<BaseGt> Point_3;
};


/// \internal
/// The Reconstruction_triangulation_3 class
/// provides the interface requested by the Implicit_reconstruction_function class:
/// - Each vertex stores a normal vector.
/// - A vertex is either an input point or a Steiner point added by Delaunay refinement.
/// - In order to solve a linear system over the triangulation, a vertex may be constrained
///   or not (i.e. may contribute to the right or left member of the linear system),
///   and has a unique index.
/// The vertex class must derive from Reconstruction_vertex_base_3.
///
/// @param BaseGt   Geometric traits class.
/// @param Gt       Geometric traits class / Point_3 is a typedef to Point_with_normal_3<BaseGt>.
/// @param Tds      Model of TriangulationDataStructure_3. The vertex class
///                 must derive from Reconstruction_vertex_base_3.

template <class BaseGt,
          class Gt = Reconstruction_triangulation_default_geom_traits_3<BaseGt>,
          class Tds_ = Triangulation_data_structure_3<Reconstruction_vertex_base_3<Gt>, Triangulation_cell_base_with_info_3<int,Gt> > >
class Reconstruction_triangulation_3 : public Delaunay_triangulation_3<Gt,Tds_>
{
// Private types
private:

  // Base class
  typedef Delaunay_triangulation_3<Gt,Tds_>  Base;

  // Auxiliary class to build an iterator over input points.
  class Is_steiner_point
  {
  public:
      typedef typename Base::Finite_vertices_iterator Finite_vertices_iterator;

      bool operator()(const Finite_vertices_iterator& v) const
      {
        return (v->type() == Reconstruction_triangulation_3::STEINER);
      }
  };

// Public types
public:

  /// Geometric traits class / Point_3 is a typedef to Point_with_normal_3<BaseGt>.
  typedef Gt  Geom_traits;

  // Repeat base class' types
  /// \cond SKIP_IN_MANUAL
  typedef Tds_ Triangulation_data_structure;
  typedef typename Base::Segment      Segment;
  typedef typename Base::Triangle     Triangle;
  typedef typename Base::Tetrahedron  Tetrahedron;
  typedef typename Base::Line         Line;
  typedef typename Base::Ray          Ray;
  typedef typename Base::Object       Object;
  typedef typename Base::Cell_handle   Cell_handle;
  typedef typename Base::Vertex_handle Vertex_handle;
  typedef typename Base::Cell   Cell;
  typedef typename Base::Vertex Vertex;
  typedef typename Base::Facet  Facet;
  typedef typename Base::Edge   Edge;
  typedef typename Base::Cell_circulator  Cell_circulator;
  typedef typename Base::Facet_circulator Facet_circulator;
  typedef typename Base::Cell_iterator    Cell_iterator;
  typedef typename Base::Facet_iterator   Facet_iterator;
  typedef typename Base::Edge_iterator    Edge_iterator;
  typedef typename Base::Vertex_iterator  Vertex_iterator;
  typedef typename Base::Point_iterator Point_iterator;
  typedef typename Base::Finite_vertices_iterator Finite_vertices_iterator;
  typedef typename Base::Finite_cells_iterator    Finite_cells_iterator;
  typedef typename Base::Finite_facets_iterator   Finite_facets_iterator;
  typedef typename Base::Finite_edges_iterator    Finite_edges_iterator;
  typedef typename Base::All_cells_iterator       All_cells_iterator;
  typedef typename Base::All_vertices_iterator       All_vertices_iterator;
  typedef typename Base::Locate_type Locate_type;
  /// \endcond

  // Geometric types
  typedef typename Geom_traits::FT FT;
  typedef typename Geom_traits::Vector_3 Vector; ///< typedef to Vector_3<BaseGt>
  typedef typename Geom_traits::Point_3 Point;  ///< typedef to Point_with_normal_3<BaseGt>
  typedef typename Geom_traits::Point_3 Point_with_normal; ///< Point_with_normal_3<BaseGt>
  typedef typename Geom_traits::Sphere_3 Sphere;
  typedef typename Geom_traits::Iso_cuboid_3 Iso_cuboid;

  typedef CGAL::Polyhedron_3<Geom_traits> Polyhedron;

  /// Point type
  enum Point_type {
    INPUT=0,    ///< Input point.
    STEINER=1   ///< Steiner point created by Delaunay refinement.
  };

  /// Iterator over input vertices.
  typedef Filter_iterator<Finite_vertices_iterator, Is_steiner_point>
                                                    Input_vertices_iterator;

  /// Iterator over input points.
  typedef Iterator_project<Input_vertices_iterator,
                           Project_point<Vertex> >  Input_point_iterator;

  mutable Sphere sphere;
  std::vector<Point_with_normal> points;
  std::size_t fraction;
  std::list<double> fractions;
  Vertex_handle constrained_vertex;


public:

  /// Default constructor.
  Reconstruction_triangulation_3()
  {}

  ~Reconstruction_triangulation_3()
  {}

  // Default copy constructor and operator =() are fine.

  // Repeat base class' public methods used below
  /// \cond SKIP_IN_MANUAL
  using Base::points_begin;
  using Base::points_end;
  using Base::number_of_vertices;
  using Base::finite_vertices_begin;
  using Base::finite_vertices_end;
  using Base::all_vertices_begin;
  using Base::all_vertices_end;

  using Base::geom_traits;
  /// \endcond

  /// Gets first iterator over input vertices.
  Input_vertices_iterator input_vertices_begin() const
  {
      return Input_vertices_iterator(finite_vertices_end(), Is_steiner_point(),
                                     finite_vertices_begin());
  }
  /// Gets past-the-end iterator over input vertices.
  Input_vertices_iterator input_vertices_end() const
  {
      return Input_vertices_iterator(finite_vertices_end(), Is_steiner_point());
  }

  /// Gets iterator over the first input point.
  Input_point_iterator input_points_begin() const
  {
      return Input_point_iterator(input_vertices_begin());
  }
  /// Gets past-the-end iterator over the input points.
  Input_point_iterator input_points_end() const
  {
      return Input_point_iterator(input_vertices_end());
  }

  /// Gets the bounding sphere of input points.


  Sphere bounding_sphere() const
  {
    return sphere;
  }

  void initialize_bounding_sphere() const
  {
    Iso_cuboid ic = bounding_box(points.begin(), points.end());
    Point center = midpoint((ic.min)(), (ic.max)());
    sphere = Sphere(center, squared_distance(center, (ic.max)()));
  }

  /// Insert point in the triangulation.
  /// Default type is INPUT.
  template <typename Visitor>
  Vertex_handle insert(const Point_with_normal& p,
                       Point_type type,// = INPUT,
                       Cell_handle start,// = Cell_handle(),
                       Visitor visitor)
  {

    if(type == INPUT){
      visitor.before_insertion();
    }
    if(this->dimension() < 3){
      Vertex_handle v = Base::insert(p, start);
      v->type() = static_cast<unsigned char>(type);
      return v;
    }
    typename Base::Locate_type lt;
    int li, lj;
    Cell_handle ch = Base::locate(p, lt, li, lj, start);

    Vertex_handle v = Base::insert(p, lt, ch, li, lj);
    v->type() = static_cast<unsigned char>(type);
    return v;
    
  }

  /// Insert the [first, beyond) range of points in the triangulation using a spatial sort.
  /// Default type is INPUT.
  ///
  /// @commentheading Template Parameters:
  /// @param PointRange is a model of `Range`. The value type of
  ///        its iterator is the key type of the named parameter `point_map`.
  /// @param PointMap is a model of `ReadablePropertyMap` with a value_type = Point_3.
  ///        It can be omitted if InputIterator value_type is convertible to Point_3.
  /// @param NormalPMap is a model of `ReadablePropertyMap` with a value_type = Vector_3.
  ///
  /// @return the number of inserted points.

  // This variant requires all parameters.
  template <typename PointRange,
            typename PointMap,
            typename NormalPMap,
            typename Visitor
  >
  int insert(
    PointRange& pts, ///< input point range
    PointMap point_map, ///< property map: `value_type of InputIterator` -> `Point_3` (the position of an input point).
    NormalPMap normal_pmap, ///< property map: `value_type of InputIterator` -> `Vector_3` (the *oriented* normal of an input point).
    Visitor visitor)
  {
    if(! points.empty()){
      std::cerr << "WARNING: not all points inserted yet" << std::endl;
    }
    // Convert input points to Point_with_normal_3
    //std::vector<Point_with_normal> points;
    for (typename PointRange::iterator it = pts.begin(); it != pts.end(); ++it)
    {
      Point_with_normal pwn(get(point_map,*it), get(normal_pmap,*it));
      points.push_back(pwn);
    }
    std::size_t n = points.size();


    initialize_bounding_sphere();

    // typedef typename PointRange::iterator Iterator_traits;
    typedef typename PointRange::difference_type Diff_t;
    boost::rand48 random;
    boost::random_number_generator<boost::rand48, Diff_t> rng(random);
    CGAL::cpp98::random_shuffle (points.begin(), points.end(), rng);
    fraction = 0;

    fractions.clear();
    fractions.push_back(1.0);
    
    double m = static_cast<double>(n);
    
    while(m > 500){
      m /= 2;
      fractions.push_front(m/n);
    }
    
    insert_fraction(visitor);
    return 0;
  }

  template <typename Visitor>
  bool insert_fraction(Visitor visitor)
  {
    if(fractions.empty()){
      points.clear();
      return false;
    }
    double frac = fractions.front();
    fractions.pop_front();
    std::size_t more = (std::size_t)(points.size() * frac) - fraction;
    if((fraction+more) > points.size()){
      more = points.size() - fraction;
    }
    Cell_handle hint;
    spatial_sort (points.begin()+fraction, points.begin()+fraction+more, geom_traits());
    for (typename std::vector<Point_with_normal>::const_iterator p = points.begin()+fraction;
         p != points.begin()+fraction+more; ++p)
    {
      Vertex_handle v = insert(*p, INPUT, hint, visitor);
      hint = v->cell();
    }
    fraction += more;
    return true;
  }

  /// \cond SKIP_IN_MANUAL
  // This variant creates a default point property map = Identity_property_map.
  template <typename PointRange,
            typename NormalPMap,
            typename Visitor
  >
  int insert(
    PointRange& pts, ///< input point range
    NormalPMap normal_pmap, ///< property map: `value_type of InputIterator` -> `Vector_3` (the *oriented* normal of an input point).
    Visitor visitor)
  {
    return insert(
      pts,
      make_identity_property_map(
      typename std::iterator_traits<typename PointRange::iterator>::value_type()),
      normal_pmap,
      visitor);
  }
  /// \endcond

  /// Delaunay refinement callback:
  /// insert STEINER point in the triangulation.
  template <class CellIt>
  Vertex_handle
  insert_in_hole(const Point_with_normal& p, CellIt cell_begin, CellIt cell_end,
	         Cell_handle begin, int i, Point_type type = STEINER)
  {
      Vertex_handle v = Base::insert_in_hole(p, cell_begin, cell_end, begin, i);
      v->type() = static_cast<unsigned char>(type);
      return v;
  }

  /// Index unconstrained vertices following the order of Finite_vertices_iterator.
  /// @return the number of unconstrained vertices.
  unsigned int index_unconstrained_vertices()
  {
    unsigned int index = 0;
    for (Finite_vertices_iterator v = finite_vertices_begin(),
         e = finite_vertices_end();
         v!= e;
         ++v)
    {
      if(! is_constrained(v))
        v->index() = index++;
    }
    return index;
  }

  unsigned int index_all_vertices()
  {
    unsigned int index = 0;
    for (Finite_vertices_iterator v = finite_vertices_begin(),
         e = finite_vertices_end();
         v!= e;
         ++v)
    {
      //if(! is_constrained(v))
        v->index() = index++;
    }
    return index;
  }

  unsigned int nb_input_vertices()
  {
	  unsigned int count = 0;
	  for (Finite_vertices_iterator v = finite_vertices_begin(),
		  e = finite_vertices_end();
		  v != e; v++)
		  if (v->type() == INPUT)
			  count++;
	
	  return count;
  }

  /// Is vertex constrained, i.e.
  /// does it contribute to the right or left member of the linear system?

  bool is_constrained(Vertex_handle v) const
  {
    return v == constrained_vertex;
  }

  void constrain(Vertex_handle v)
  {
    constrained_vertex = v;
  }

  /// Marching Tets
  template <class Point_3, class Polygon_3>
  unsigned int marching_tets(const FT value, 
                             //const std::string outfile,
                             std::ofstream& out,
                             std::vector< Point_3 >& m_contour_points,
                             std::vector< Polygon_3 >& m_contour_polygons)
  {
    unsigned int nb_tri = 0;
    Finite_cells_iterator v, e; 

    for(v = this->finite_cells_begin(),
        e = this->finite_cells_end();
        v != e;
        ++v)
        nb_tri += contour(v, value, m_contour_points, m_contour_polygons);

    Polyhedron mesh;
    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(m_contour_points, m_contour_polygons, mesh);
    //if (CGAL::is_closed(mesh) && (!CGAL::Polygon_mesh_processing::is_outward_oriented(mesh)))
    //  CGAL::Polygon_mesh_processing::reverse_face_orientations(mesh);

    out << mesh;
    out.close();

    return nb_tri;
  }

  template <class Point_3, class Polygon_3>
  unsigned int contour(Cell_handle cell, const FT value, 
                        std::vector< Point_3 >& m_pts, 
                        std::vector< Polygon_3 >& m_polys)
  {
    std::list<Point> cell_points;
    Vector direction;

    if(!extract_level_set_points(cell, value, cell_points, direction))
      return 0;

    if(cell_points.size() == 3)
    {
      typename std::list<Point>::iterator it = cell_points.begin();
      const Point& a = (*it); it++;
      const Point& b = (*it); it++;
      const Point& c = (*it);

      Vector n = CGAL::cross_product((b-a), (c-a));

      m_pts.push_back(a);
      m_pts.push_back(b);
      m_pts.push_back(c);

      if(n * direction >= 0){
        std::vector<std::size_t> m_idx{m_pts.size()-3, m_pts.size()-2, m_pts.size()-1};
        m_polys.push_back(m_idx);
      } 
      else{
        std::vector<std::size_t> m_idx{m_pts.size()-3, m_pts.size()-1, m_pts.size()-2};
        m_polys.push_back(m_idx);        
      }
      return 1;
    }
    else if(cell_points.size() == 4)
    {
      typename std::list<Point>::iterator it = cell_points.begin();
      std::vector<Point> p(4);
      for(int i = 0; i < 4; i++)
      {
        p[i] = (*it);
        it++;
      }
      // compute normal
      Vector u = p[1] - p[0];
      Vector v = p[2] - p[0];
      Vector n = CGAL::cross_product(u, v);

      m_pts.push_back(p[0]);
      m_pts.push_back(p[1]);
      m_pts.push_back(p[2]);
      m_pts.push_back(p[3]);

      if(n * direction <= 0){
        std::vector<std::size_t> m_idx_1{m_pts.size()-4, m_pts.size()-2, m_pts.size()-1},
                                 m_idx_2{m_pts.size()-4, m_pts.size()-1, m_pts.size()-3};
        m_polys.push_back(m_idx_1);
        m_polys.push_back(m_idx_2);
      }
      else{
        std::vector<std::size_t> m_idx_1{m_pts.size()-4, m_pts.size()-3, m_pts.size()-1},
                                 m_idx_2{m_pts.size()-4, m_pts.size()-1, m_pts.size()-2};
        m_polys.push_back(m_idx_1);
        m_polys.push_back(m_idx_2);
      }
      return 2;
    }
    return 0;   
  }

  bool extract_level_set_points(Cell_handle cell, const FT value, std::list<Point>& points, Vector& direction)
  {
    Point point;
    if(level_set(cell,value,0,1,point, direction)) points.push_back(point);
		if(level_set(cell,value,0,2,point, direction)) points.push_back(point);
		if(level_set(cell,value,0,3,point, direction)) points.push_back(point);
		if(level_set(cell,value,1,2,point, direction)) points.push_back(point);
		if(level_set(cell,value,1,3,point, direction)) points.push_back(point);
		if(level_set(cell,value,2,3,point, direction)) points.push_back(point);
		return points.size() != 0;
  }

  bool level_set(Cell_handle cell, const FT value, const int i1, const int i2, Point& p, Vector& direction)
  {
    const Point& p1 = cell->vertex(i1)->point();
    const Point& p2 = cell->vertex(i2)->point();
    double v1 = cell->vertex(i1)->f();
    double v2 = cell->vertex(i2)->f();

    if(v1 <= value && v2 >= value)
    {
      double ratio = (value - v1) / (v2 - v1);
      p = p1 + ratio * (p2 - p1);
      direction = p2 - p1;
      return true;
    }
    else if(v2 <= value && v1 >= value)
    {
      double ratio = (value - v2) / (v1 - v2);
      p = p2 + ratio * (p1 - p2);
      direction = p1 - p2;
      return true;
    }
    return false;
  }

}; // end of Reconstruction_triangulation_3

} //namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // CGAL_IMPLICIT_FCT_DELAUNAY_TRIANGULATION_H
