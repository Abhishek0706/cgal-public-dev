// Copyright (c) 2009 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://mbogdanov@scm.gforge.inria.fr/svn/cgal/trunk/Mesh_3/include/CGAL/Mesh_facet_criteria_3.h $
// $Id: Mesh_facet_criteria_3.h 60688 2011-01-10 15:43:22Z lrineau $
//
//
// Author(s)     : Stéphane Tayeb
//
//******************************************************************************
// File Description :
// Mesh_facet_criteria_3 class.
//******************************************************************************

#ifndef CGAL_MESH_FACET_CRITERIA_3_H
#define CGAL_MESH_FACET_CRITERIA_3_H

#include <CGAL/Periodic_mesh_3/periodic_mesh_standard_facet_criteria.h>
#include <CGAL/Mesh_facet_topology.h>

namespace CGAL {
  
template<typename Tr,
  typename Visitor_ = Mesh_3::Periodic_mesh_3::Facet_criterion_visitor_with_features<Tr> >
class Mesh_facet_criteria_3
{
public:
  typedef Visitor_ Visitor;
  typedef typename Visitor::Facet_quality Facet_quality;
  typedef typename Visitor::Facet_badness Facet_badness;
  
private:
  typedef Mesh_3::Criteria<Tr,Visitor> Criteria;
  typedef Mesh_3::Abstract_criterion<Tr,Visitor> Abstract_criterion;

  typedef typename Tr::Facet Facet;
  typedef typename Tr::Geom_traits::FT FT;
  
  typedef typename Tr::Iso_cuboid Iso_cuboid;

  typedef Mesh_facet_criteria_3<Tr> Self;

public:
  /**
   * @brief Constructor
   */
  
  Mesh_facet_criteria_3(const Iso_cuboid& iso_cuboid,
                        const FT& angle_bound,
                        const FT& radius_bound,
                        const FT& distance_bound,
                        const Mesh_facet_topology topology =
                        FACET_VERTICES_ON_SURFACE) :
    helper_(iso_cuboid)
  {
    if ( FT(0) != angle_bound )
      init_aspect(angle_bound);
    
    if ( FT(0) != radius_bound )
      init_radius(radius_bound);
    
    if ( FT(0) != distance_bound )
      init_distance(distance_bound);
    
    init_topo(topology);
  }
  
  template< typename MD >
  Mesh_facet_criteria_3(const MD& periodic_domain,
                        const FT& angle_bound,
                        const FT& radius_bound,
                        const FT& distance_bound,
                        const Mesh_facet_topology topology =
                        FACET_VERTICES_ON_SURFACE) :
    helper_(periodic_domain.periodic_cuboid())
  {
    if ( FT(0) != angle_bound )
      init_aspect(angle_bound);
    
    if ( FT(0) != radius_bound )
      init_radius(radius_bound);
    
    if ( FT(0) != distance_bound )
      init_distance(distance_bound);
    
    init_topo(topology);
  }

  // Nb: SFINAE (dummy) to avoid wrong matches with built-in numerical types
  // as int.
  template < typename MD, typename Sizing_field >
  Mesh_facet_criteria_3(const MD& periodic_domain,
                        const FT& angle_bound,
                        const Sizing_field& radius_bound,
                        const FT& distance_bound,
                        const Mesh_facet_topology topology = 
                          FACET_VERTICES_ON_SURFACE,
                        typename Sizing_field::FT dummy = 0) :
    helper_(periodic_domain)
  {
    if ( FT(0) != angle_bound )
      init_aspect(angle_bound);
    
    init_radius(radius_bound);
    
    if ( FT(0) != distance_bound )
      init_distance(distance_bound);
    
    init_topo(topology);  
  }
  
  /// Destructor
  ~Mesh_facet_criteria_3() { }

   /**
   * @brief returns the badness of facet \c facet
   * @param facet the facet
   * @return the badness of \c facet
   */
  Facet_badness operator()(const Facet& facet) const
  {
    return criteria_(facet);
  }
  
  void add(Abstract_criterion* criterion)
  {
    criteria_.add(criterion);
  }

private:
  void init_aspect(const FT& angle_bound)
  {
    typedef Mesh_3::Periodic_mesh_3::Aspect_ratio_criterion<Tr,Visitor> Aspect_criterion;
    criteria_.add(new Aspect_criterion(helper_, angle_bound));
  }
  
  void init_radius(const FT& radius_bound)
  {
    typedef Mesh_3::Periodic_mesh_3::Uniform_size_criterion<Tr,Visitor> Uniform_size_criterion;
    criteria_.add(new Uniform_size_criterion(helper_, radius_bound));
  }
  
  template <typename Sizing_field>
  void init_radius(const Sizing_field& radius_bound)
  {
    typedef Mesh_3::Periodic_mesh_3::Variable_size_criterion<Tr,Visitor,Sizing_field> Variable_size_criterion;
    criteria_.add(new Variable_size_criterion(radius_bound));
  }
  
  void init_distance(const FT& distance_bound)
  {
    typedef Mesh_3::Periodic_mesh_3::Curvature_size_criterion<Tr,Visitor> Curvature_criterion;
    criteria_.add(new Curvature_criterion(helper_, distance_bound));
  }
  
  void init_topo(const Mesh_facet_topology topology)
  {
    switch ( topology )
    {
      case FACET_VERTICES_ON_SURFACE:
      {
        typedef Mesh_3::Periodic_mesh_3::Facet_on_surface_criterion<Tr,Visitor> On_surface_criterion;
        criteria_.add(new On_surface_criterion());
        break;
      }
        
      case FACET_VERTICES_ON_SAME_SURFACE_PATCH:
      {
        typedef Mesh_3::Periodic_mesh_3::Facet_on_same_surface_criterion<Tr,Visitor> Same_surface_criterion;
        criteria_.add(new Same_surface_criterion());
        break;
      }
      
      case FACET_VERTICES_ON_SAME_SURFACE_PATCH_WITH_ADJACENCY_CHECK:
      {
        // @TODO: Implement adjacency check !
        typedef Mesh_3::Periodic_mesh_3::Facet_on_same_surface_criterion<Tr,Visitor> Same_surface_criterion;
        criteria_.add(new Same_surface_criterion());
        break;
      }
    }
  }
  
private:
  Criteria criteria_;
  
  Tr helper_;
};  // end class Mesh_facet_criteria_3

}  // end namespace CGAL


#endif // CGAL_MESH_FACET_CRITERIA_3_H
