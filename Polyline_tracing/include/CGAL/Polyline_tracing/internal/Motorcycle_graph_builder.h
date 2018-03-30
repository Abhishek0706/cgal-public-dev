// Copyright (c) 2018 GeometryFactory (France).
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
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_POLYLINE_TRACING_MOTORCYCLE_GRAPH_BUILDER_H
#define CGAL_POLYLINE_TRACING_MOTORCYCLE_GRAPH_BUILDER_H

#include <CGAL/assertions.h>
#include <CGAL/boost/graph/iterator.h>

#include <CGAL/Polygon_mesh_processing/locate.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace CGAL {

namespace Polyline_tracing {

namespace internal {

// @tmp temporary halfedgegraph is_valid()
template <typename HDS>
bool is_valid_hds(const HDS& g)
{
  typedef typename boost::graph_traits<HDS>::halfedge_descriptor   halfedge_descriptor;
  typedef typename boost::graph_traits<HDS>::vertex_descriptor     vertex_descriptor;
  typedef typename boost::graph_traits<HDS>::vertices_size_type    vertex_size_type;
  typedef typename boost::graph_traits<HDS>::halfedges_size_type   halfedges_size_type;

  std::size_t num_v(std::distance(boost::begin(vertices(g)), boost::end(vertices(g)))),
              num_h(std::distance(boost::begin(halfedges(g)), boost::end(halfedges(g))));

  bool valid = (1 != (num_h&1));
  if(!valid)
    std::cerr << "number of halfedges is odd." << std::endl;

  // All halfedges.
  halfedges_size_type n = 0;
  halfedges_size_type nb = 0;
  BOOST_FOREACH(halfedge_descriptor begin, halfedges(g))
  {
    if(!valid)
      break;

    // Pointer integrity.
    valid = valid && (next(begin, g) != boost::graph_traits<HDS>::null_halfedge());
    valid = valid && (opposite(begin, g) != boost::graph_traits<HDS>::null_halfedge());
    if(!valid)
    {
      std::cerr << "pointer integrity corrupted (ptr==0)." << std::endl;
      break;
    }

    // edge integrity
    valid = valid && (halfedge(edge(begin, g), g) == begin);

    // opposite integrity.
    valid = valid && (opposite(begin, g) != begin);
    valid = valid && (opposite(opposite(begin, g), g) == begin);
    if(!valid)
    {
      std::cerr << "opposite pointer integrity corrupted." << std::endl;
      break;
    }

    // previous integrity.
    valid = valid && (prev(next(begin, g), g) == begin);
    valid = valid && (next(prev(begin, g), g) == begin);
    if(!valid)
    {
      std::cerr << "previous pointer integrity corrupted." << std::endl;
      break;
    }

    // vertex integrity.
    valid = valid && (target(begin, g) != boost::graph_traits<HDS>::null_vertex());
    if(!valid)
    {
      std::cerr << "vertex pointer integrity corrupted." << std::endl;
      break;
    }
    valid = valid && (target(begin, g) == target(opposite(next(begin, g), g), g));
    if(!valid)
    {
      std::cerr << "vertex pointer integrity2 corrupted." << std::endl;
      break;
    }
    // face integrity.

    valid = valid && ( face(begin, g) == face(next(begin, g), g));
    if(!valid)
    {
      std::cerr << "face pointer integrity2 corrupted." << std::endl;
      break;
    }

    ++n;
    if(is_border(begin, g))
      ++nb;
  }

  if(valid && n != num_h)
    std::cerr << "counting halfedges failed." << std::endl;

  // All vertices.
  vertex_size_type v = 0;
  n = 0;
  BOOST_FOREACH(vertex_descriptor vbegin, vertices(g))
  {
    if(!valid)
      break;

    // Pointer integrity.
    if(halfedge(vbegin, g) != boost::graph_traits<HDS>::null_halfedge())
      valid = valid && (target( halfedge(vbegin, g), g) == vbegin);
    else
      valid = false;

    if(!valid)
    {
      std::cerr << "halfedge pointer in vertex corrupted." << std::endl;
      break;
    }

    // cycle-around-vertex test.
    halfedge_descriptor h = halfedge(vbegin, g);
    if( h != boost::graph_traits<HDS>::null_halfedge())
    {
      halfedge_descriptor ge = h;
      do
      {
        ++n;
        h = opposite(next(h, g), g);
        valid = valid && (n <= num_h && n!=0);
        if(!valid)
          std::cerr << "too many halfedges around vertices." << std::endl;
      }
      while(valid && (h != ge));
    }
    ++v;
  }

  if( valid && v != num_v)
    std::cerr << "counting vertices failed." << std::endl;
  if( valid && ( n  != num_h))
    std::cerr << "counting halfedges via vertices failed." << std::endl;
  valid = valid && ( v == num_v);

  std::cerr << "end of CGAL::is_valid_polygon_mesh(): structure is "
            << (valid ? "valid." : "NOT VALID.") << std::endl;

  return valid;
}

enum Point_output_selection
{
  ALL_POINTS,
  DESTINATIONS_AND_COLLISIONS // do not output intermediary points
};

// to reduce the size of the 'Motorcycle Graph' class
template<typename Motorcycle_graph>
struct Motorcycle_graph_builder
{
  typedef typename Motorcycle_graph::Geom_traits                            Geom_traits;
  typedef typename Motorcycle_graph::Triangle_mesh                          Triangle_mesh; // input mesh
  typedef typename Motorcycle_graph::Halfedge_graph                         Halfedge_graph; // output graph

  typedef typename Motorcycle_graph::Node_ptr                               Node_ptr;

  typedef typename boost::graph_traits<Triangle_mesh>::vertex_descriptor    vertex_descriptor;
  typedef typename boost::graph_traits<Triangle_mesh>::halfedge_descriptor  halfedge_descriptor;
  typedef typename boost::graph_traits<Triangle_mesh>::face_descriptor      face_descriptor;
  typedef boost::variant<vertex_descriptor,
                         halfedge_descriptor,
                         face_descriptor>                                   descriptor_variant;

  typedef typename boost::graph_traits<Halfedge_graph>::vertex_descriptor   hg_vertex_descriptor;
  typedef typename boost::graph_traits<Halfedge_graph>::halfedge_descriptor hg_halfedge_descriptor;
  typedef typename boost::graph_traits<Halfedge_graph>::edge_descriptor     hg_edge_descriptor;
  typedef typename boost::graph_traits<Halfedge_graph>::face_descriptor     hg_face_descriptor;

  typedef typename Geom_traits::Point_d                                     Point;
  typedef typename Geom_traits::Point_2                                     Point_2;
  typedef typename Geom_traits::Direction_2                                 Direction_2;
  typedef typename Geom_traits::Line_2                                      Line_2;

  typedef typename Motorcycle_graph::Motorcycle                             Motorcycle;
  typedef typename Motorcycle_graph::Track_segment                          Track_segment;
  typedef typename Motorcycle_graph::Track                                  Track;

  Motorcycle_graph_builder(Motorcycle_graph& mg,
                           const Point_output_selection point_selection = ALL_POINTS,
                           const bool map_vertices = true,
                           const bool color_edges = true)
    : mg(mg), og(mg.graph()),
      point_selection(point_selection),
      map_vertices(map_vertices),
      color_edges(color_edges)
  { }

private:
  struct Incident_edge
  {
    Incident_edge(const hg_halfedge_descriptor hd,
                  const face_descriptor fd,
                  const Direction_2 d)
      :
        ihd(hd), fd(fd), dir(d)
    { }

    hg_halfedge_descriptor ihd; // incident halfedge
    face_descriptor fd; // face on which the halfedge is
    Direction_2 dir; // direction_2 in the local reference frame of the face
  };

  struct Incident_edges
  {
    Incident_edges(const Node_ptr p) : p(p), iedges() { }

    Node_ptr p; // extremity common to all the edges
    std::vector<Incident_edge> iedges;
  };

  template <typename PointDescriptorMap, typename VertexPointMap, typename VertexDictionaryMap>
  hg_vertex_descriptor create_vertex(Node_ptr it,
                                     PointDescriptorMap& pdm,
                                     VertexPointMap vpm,
                                     VertexDictionaryMap vdm)
  {
    std::pair<typename PointDescriptorMap::iterator, bool> is_insert_successful =
      pdm.insert(std::make_pair(it->point(), boost::graph_traits<Halfedge_graph>::null_vertex()));

    if(is_insert_successful.second)
    {
      hg_vertex_descriptor vd = add_vertex(og);
      it->vertex() = vd;

      is_insert_successful.first->second = vd;
      put(vpm, vd, it->point());

      if(map_vertices)
        put(vdm, vd, it);

      return vd;
    }
    else
    {
      return is_insert_successful.first->second;
    }
  }

  template <typename VIMap>
  void add_incident_track_to_vertex(const hg_vertex_descriptor vd,
                                    const hg_halfedge_descriptor hd,
                                    const Node_ptr s_it, // source and target of hd
                                    const Node_ptr t_it,
                                    VIMap& vim) const
  {
    CGAL_precondition(target(hd, og) == vd);
//    CGAL_precondition(t_it->point() == og.point(vd)); // @fixme needs pmap and converter
    face_descriptor fd = s_it->face();
    CGAL_precondition(fd == t_it->face());

//    CGAL_precondition(og.point(vd) != s_it->point());
//    CGAL_precondition(og.point(vd) == t_it->point());

    Incident_edges inc_edges(t_it);
    std::pair<typename VIMap::iterator, bool> is_insert_successful =
      vim.insert(std::make_pair(vd, inc_edges));

    const Point_2 s = mg.geom_traits().construct_point_2_object()(s_it->location().second[0],
                                                                  s_it->location().second[1]);
    const Point_2 t = mg.geom_traits().construct_point_2_object()(t_it->location().second[0],
                                                                  t_it->location().second[1]);
    const Line_2 l = mg.geom_traits().construct_line_2_object()(s, t);
    const Direction_2 d = mg.geom_traits().construct_direction_2_object()(l);

    is_insert_successful.first->second.iedges.push_back(Incident_edge(hd, fd, d));
  }

  struct Edge_global_order
  {
    bool operator()(const Incident_edge& e1, const Incident_edge& e2) const
    {
      if(e1.fd == e2.fd)
        return e1.dir > e2.dir; // compare the directions if within the same face...
      return (e1.fd < e2.fd); // ...otherwise, compare the directions
    }
  };

  template <typename IncidentEdgeInputIterator, typename IncidentEdgeOutputIterator>
  IncidentEdgeOutputIterator order_incident_edges_in_face(const Node_ptr v,
                                                          IncidentEdgeInputIterator first,
                                                          IncidentEdgeInputIterator last,
                                                          IncidentEdgeOutputIterator out) const
  {
    CGAL_precondition(v != Node_ptr());
    CGAL_precondition(first != last);

    namespace PMP = CGAL::Polygon_mesh_processing;

    // If the motorcycle graph vertex is on a face or an edge, ordering by face
    // and internally within each face is enough to have a global ordering.
    // However, if it is on a mesh vertex, then the incident faces must also
    // be ordered.
    //
    // The trick is that the local ordering on each face has some global consistency
    // because we are ordering within each face according to 2D directions which
    // are computed using barycentric coordinates. But, the barycentric frame
    // is chosen the same way on each face and thus has a global consistency!
    //
    // Thus: local ordering of edges on each face + global ordering the faces = global ordering of the edges
    std::sort(first, last, Edge_global_order());

    descriptor_variant dv = PMP::get_descriptor_from_location(v->location(), mg.mesh());
    if(const vertex_descriptor* vd_ptr = boost::get<vertex_descriptor>(&dv))
    {
      const vertex_descriptor& vd = *vd_ptr;

      // Read the partially sorted range and mark down the iterators at which we start new faces
      typedef boost::unordered_map<face_descriptor, IncidentEdgeInputIterator>  Iterator_position_map;
      Iterator_position_map face_positions;

      IncidentEdgeInputIterator ieit = first;
      face_descriptor current_fd = ieit->fd;
      face_positions[current_fd] = ieit;
      while(ieit != last)
      {
        if(current_fd != ieit->fd)
        {
          current_fd = ieit->fd;
          face_positions[current_fd] = ieit;
        }

        ++ieit;
      }

      // Create a completely ordered edge range by traversing the input range
      // in the same order that the faces are ordered around the vertex
      CGAL::Face_around_target_iterator<Triangle_mesh> fit, fend;
      boost::tie(fit, fend) = CGAL::faces_around_target(halfedge(vd, mg.mesh()), mg.mesh());

      CGAL_assertion(face_positions.size() <= static_cast<std::size_t>(std::distance(fit, fend)));

      while(fit != fend)
      {
        current_fd = *fit++;

        if(current_fd == boost::graph_traits<Triangle_mesh>::null_face())
          continue;

        typename Iterator_position_map::iterator pos = face_positions.find(current_fd);
        if(pos == face_positions.end())
          continue;

        IncidentEdgeInputIterator face_position = pos->second;
        while(face_position->fd == current_fd)
        {
          *out++ = face_position->ihd;
          ++face_position;
        }
      }
    }
    else // point is not on a vertex, simply dump the already ordered edges
    {
      while(first != last)
      {
        *out++ = first->ihd;
        ++first;
      }
    }

    return out;
  }

  template <typename VIMap>
  void setup_graph_incidences(VIMap& vim)
  {
    typename boost::graph_traits<Halfedge_graph>::vertex_iterator vit, vend;
    boost::tie(vit, vend) = vertices(og);

    while(vit != vend)
    {
      hg_vertex_descriptor vd = *vit++;
      typename VIMap::iterator pos = vim.find(vd);

      if(pos == vim.end())
      {
        std::cerr << "Warning: vertex not in the VIMap" << std::endl;
        continue;
      }

      Incident_edges& incident_edges = pos->second;
      typename std::vector<Incident_edge>::iterator lit = incident_edges.iedges.begin(),
                                                    end = incident_edges.iedges.end();

      // degenerate case of a single incident edge
      if(std::distance(lit, end) == 1)
      {
        hg_halfedge_descriptor hd = lit->ihd;
        set_next(hd, opposite(hd, og), og);
        continue;
      }

      std::vector<hg_halfedge_descriptor> ordered_halfedges;
      order_incident_edges_in_face(incident_edges.p, lit, end, std::back_inserter(ordered_halfedges));

      typename std::vector<hg_halfedge_descriptor>::const_iterator hd_cit = ordered_halfedges.begin(),
                                                                   hd_last = --(ordered_halfedges.end()),
                                                                   hd_end = ordered_halfedges.end();
      for(; hd_cit!=hd_end; ++hd_cit)
      {
        hg_halfedge_descriptor current_hd = *hd_cit;
        hg_halfedge_descriptor next_hd = *((hd_cit == hd_last) ? ordered_halfedges.begin() : CGAL::cpp11::next(hd_cit));

        CGAL_assertion(target(current_hd, og) == vd);
        CGAL_assertion(target(next_hd, og) == vd);

        set_next(current_hd, opposite(next_hd, og), og);
      }
    }
  }

public:
  // @todo can probably rely a little less on maps to make it faster but it's
  // likely to be a very cheap function anyway and it's better if it's readable.
  bool operator()()
  {
    std::cout << "Constructing motorcycle graph..." << std::endl;

    CGAL_precondition(num_vertices(og) == 0 && num_edges(og) == 0);
    CGAL_assertion(point_selection == ALL_POINTS); // other setting are not currently supported

    typedef typename boost::property_map<Halfedge_graph, CGAL::dynamic_vertex_property_t<Node_ptr> >::type VertexDictionaryMap;

    typedef int                                                                                          Color;
    typedef typename boost::property_map<Halfedge_graph, CGAL::dynamic_edge_property_t<Color> >::type    EdgeColorMap;

    CGAL_static_assertion((CGAL::graph_has_property<Halfedge_graph, boost::vertex_point_t>::value));
    typedef typename property_map_selector<Halfedge_graph, CGAL::vertex_point_t>::type                   VPMap;

    VertexDictionaryMap vdm;
    if(map_vertices)
      vdm = get(CGAL::dynamic_vertex_property_t<Node_ptr>(), og);

    EdgeColorMap ecm;
    if(color_edges)
      ecm = get(CGAL::dynamic_edge_property_t<Color>(), og);

    VPMap vpm = get_property_map(boost::vertex_point, og);

    // Associate to a point the corresponding vertex_descriptor in the graph
    typedef std::map<Point, hg_vertex_descriptor>                               VDMap;

    // Associate to each vertex a list of incident halfedges, the face in which they are,
    // and a 2D direction to order them around the vertex
    typedef boost::unordered_map<hg_vertex_descriptor, Incident_edges>          VIMap;

    VDMap vds;
    VIMap vim;

    // - points.size() is slightly greater than the number of useful vertices (due to the border)
    // - there are roughly two halfedges per vertex
    // - there are no faces
    typename boost::graph_traits<Halfedge_graph>::vertices_size_type nv(mg.nodes().size());
    typename boost::graph_traits<Halfedge_graph>::vertices_size_type nh(2 * mg.nodes().size());
    reserve(og, nv, nh, 0);

    // First: create all the vertices, halfedges, and edges
    const std::size_t nm = mg.number_of_motorcycles();
    for(std::size_t mc_id = 0; mc_id<nm; ++mc_id)
    {
      Motorcycle& mc = mg.motorcycle(mc_id);
      Track& mct = mc.track();

      if(mct.size() == 0)
      {
        std::cout << "Warning: motorcycle " << mc_id << " has no track" << std::endl;
        CGAL_assertion(false);
        continue;
      }

      Track_segment& first_ts = mct.front();
      hg_vertex_descriptor current_vd = create_vertex(first_ts.source(), vds, vpm, vdm);

      typename Track::iterator tscit = mct.begin(), end = mct.end();
      for(; tscit!=end; ++tscit)
      {
        Track_segment& ts = *tscit;

        Node_ptr track_source = ts.source();
        Node_ptr track_target = ts.target();

        hg_vertex_descriptor next_vd = boost::graph_traits<Halfedge_graph>::null_vertex();
        if(track_source == track_target || track_source->point() == track_target->point())
        {
          std::cerr << "Warning: degenerate track at: " << track_source->point() << std::endl;
          next_vd = current_vd;
          continue;
        }
        else
        {
          next_vd = create_vertex(track_target, vds, vpm, vdm);

          // Create the new edge
          hg_edge_descriptor ed = add_edge(og);
          ts.edge() = ed;

          if(color_edges)
            put(ecm, ed, mc_id);

          hg_halfedge_descriptor hd = halfedge(ed, og);
          set_target(hd, next_vd, og);
          hg_halfedge_descriptor opp_hd = opposite(hd, og);
          set_target(opp_hd, current_vd, og);

          set_halfedge(next_vd, hd, og);
          set_halfedge(current_vd, opp_hd, og);
          CGAL_assertion(target(hd, og) == next_vd);
          CGAL_assertion(target(opp_hd, og) == current_vd);

          // Fill the incident map
          add_incident_track_to_vertex(current_vd, opp_hd, track_target, track_source, vim);
          add_incident_track_to_vertex(next_vd, hd, track_source, track_target, vim);
        }

        current_vd = next_vd;
      }
    }

    // Second: set up the adjacency halfedge relationships (prev/next)
    setup_graph_incidences(vim);

    // Third: create faces ? There are cycles now... @todo (also change reserve with Euler?)

    return is_valid_hds(og);
  }

private:
  Motorcycle_graph& mg;
  Halfedge_graph& og;

  // map_vertices: can be useful to have a map between the vertex descriptor of the output graph
  //               and the points of the dictionary
  // color edges: each edge is the track of a single motorcycle and we can associate a color to it
  const Point_output_selection point_selection;
  const bool map_vertices = true;
  const bool color_edges = true;
};

} // namespace internal

} // namespace Polyline_tracing

} // namespace CGAL

#endif // CGAL_POLYLINE_TRACING_MOTORCYCLE_GRAPH_BUILDER_H
