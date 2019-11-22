// Copyright (c) 2019 CNRS and LIRIS' Establishments (France).
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
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_CURVES_ON_SURFACE_TOPOLOGY_H
#define CGAL_CURVES_ON_SURFACE_TOPOLOGY_H 1

#include <CGAL/license/Surface_mesh_topology.h>

#include <CGAL/Surface_mesh_topology/internal/Minimal_quadrangulation.h>
#include <CGAL/Face_graph_wrapper.h>
#include <memory>

namespace CGAL {
namespace Surface_mesh_topology {

template<typename Mesh>
class Curves_on_surface_topology
{
public:
  typedef typename internal::CMap_for_minimal_quadrangulation
  CMap_for_minimal_quadrangulation;
  
  Curves_on_surface_topology(const Mesh& amap, bool /* display_time */=false) :
    m_original_map(amap),
    m_minimal_quadrangulation(nullptr)
  {}

  /// @return true iff the minimal quadrangulation is computed.
  bool is_minimal_quadrangulation_computed() const
  { return m_minimal_quadrangulation!=nullptr; }

  /// Computes the minimal quadrangulation if it is not yet computed.
  void compute_minimal_quadrangulation(bool display_time=true) const
  {
    if (m_minimal_quadrangulation==nullptr)
    {
      m_minimal_quadrangulation.reset
          (new internal::Minimal_quadrangulation<Mesh>
           (m_original_map, display_time));
    }
  }

  /// Return the reduced map computed in the minimal quadrangulation.
  /// @pre is_minimal_quadrangulation_computed()
  const CMap_for_minimal_quadrangulation& get_minimal_quadrangulation() const
  {
    CGAL_assertion(is_minimal_quadrangulation_computed());
    return m_minimal_quadrangulation->get_map();
  }

  /// @return true iff 'p' is contractible.
  bool is_contractible(const Path_on_surface<Mesh>& p,
                       bool display_time=false) const
  {
    compute_minimal_quadrangulation(display_time);
    return m_minimal_quadrangulation->is_contractible(p, display_time);
  }

  /// @return true iff 'p1' and 'p2' are freely homotopic.
  bool are_freely_homotopic(const Path_on_surface<Mesh>& p1,
                            const Path_on_surface<Mesh>& p2,
                            bool display_time=false) const
  {
    compute_minimal_quadrangulation(display_time);
    return m_minimal_quadrangulation->are_freely_homotopic(p1, p2,
                                                           display_time);
  }
  
  /// @return true iff 'p1' and 'p2' are base point freely homotopic.
  bool are_base_point_homotopic(const Path_on_surface<Mesh>& p1,
                                const Path_on_surface<Mesh>& p2,
                                bool display_time=false) const
  {
    compute_minimal_quadrangulation(display_time);
    return m_minimal_quadrangulation->are_base_point_homotopic(p1, p2,
                                                               display_time);
  }

protected:
  const Mesh& m_original_map;
  mutable std::unique_ptr<internal::Minimal_quadrangulation<Mesh>>
  m_minimal_quadrangulation;
};

} // namespace Surface_mesh_topology
} // namespace CGAL

#endif // CGAL_CURVES_ON_SURFACE_TOPOLOGY_H //
// EOF //
