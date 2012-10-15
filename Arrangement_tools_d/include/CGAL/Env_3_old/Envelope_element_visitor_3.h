// Copyright (c) 2005  Tel-Aviv University (Israel).
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
// $Source: /CVSROOT/CGAL/Packages/Envelope_3/include/CGAL/Envelope_element_visitor_3.h,v $
// $Revision$ $Date$
// $Name:  $
//
// Author(s)     : Michal Meyerovitch     <gorgymic@post.tau.ac.il>

#ifndef CGAL_ENVELOPE_ELEMENT_VISITOR_3_H
#define CGAL_ENVELOPE_ELEMENT_VISITOR_3_H

#include <CGAL/Object.h>
#include <CGAL/enum.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/Arr_observer.h>
#include <CGAL/Arr_accessor.h>
#include <CGAL/Arr_walk_along_line_point_location.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arrangement_2/Arr_inc_insertion_zone_visitor.h>
#include <CGAL/Arrangement_2_incremental_insert.h>
#include <CGAL/Timer.h>
#include <CGAL/utility.h>

#include <vector>
#include <algorithm>
#include <iostream>
#include <deque>

namespace CGAL {

// this class does the resolving of edge and face in the divide & conquer algorithm
// it should handle all faces (it supports holes in the face)

template <class EnvelopeTraits_3, class MinimizationDiagram_2>
class Envelope_element_visitor_3
{
public:
  typedef EnvelopeTraits_3                             Traits;
  typedef typename Traits::Surface_3                   Surface_3;
  typedef typename Traits::Xy_monotone_surface_3       Xy_monotone_surface_3;

  typedef MinimizationDiagram_2                        Minimization_diagram_2;
  typedef typename Traits::Point_2                     Point_2;
  typedef typename Traits::X_monotone_curve_2          X_monotone_curve_2;
  typedef typename Traits::Curve_2                     Curve_2;

protected:

  typedef Envelope_element_visitor_3<Traits, Minimization_diagram_2> Self;
  typedef typename Minimization_diagram_2::Halfedge_const_iterator  Halfedge_const_iterator;
  typedef typename Minimization_diagram_2::Halfedge_handle          Halfedge_handle;
  typedef typename Minimization_diagram_2::Halfedge_iterator        Halfedge_iterator;
  typedef typename Minimization_diagram_2::Face_handle              Face_handle;
  typedef typename Minimization_diagram_2::Face_iterator            Face_iterator;
  typedef typename Minimization_diagram_2::Vertex_handle            Vertex_handle;
  typedef typename Minimization_diagram_2::Vertex_iterator          Vertex_iterator;
  typedef typename Minimization_diagram_2::Ccb_halfedge_circulator  Ccb_halfedge_circulator;
  typedef typename Minimization_diagram_2::Hole_iterator            Hole_iterator;
  typedef typename Minimization_diagram_2::Isolated_vertex_iterator Isolated_vertex_iterator;
  typedef typename Minimization_diagram_2::Dcel                     Dcel;
  typedef typename Minimization_diagram_2::Dcel::Dcel_data_iterator Envelope_data_iterator;

  typedef Arr_observer<Minimization_diagram_2>                      Md_observer;
  typedef Arr_accessor<Minimization_diagram_2>                      Md_accessor;
  
  typedef Arr_walk_along_line_point_location<Minimization_diagram_2>
                                                                    Md_point_location;

  typedef Arr_inc_insertion_zone_visitor<Minimization_diagram_2>    Md_insert_zone_visitor;
  
  typedef std::list<Halfedge_handle>                                Halfedges_list;
  typedef typename std::list<Halfedge_handle>::iterator             Halfedges_list_iterator;

  typedef std::pair<Halfedge_handle, Intersection_type>             Halfedge_w_type;
  typedef std::list<Halfedge_w_type>                                Halfedges_w_type_list;

  typedef std::list<Vertex_handle>                                  Vertices_list;
  typedef typename std::list<Vertex_handle>::iterator               Vertices_list_iterator;

  typedef std::list<Face_handle>                                    Faces_list;
  typedef typename std::list<Face_handle>::iterator                 Faces_list_iterator;

  typedef Unique_hash_map<Vertex_handle, bool>                      Vertices_hash;
  typedef Unique_hash_map<Halfedge_handle, bool>                    Halfedges_hash;
  typedef Unique_hash_map<Halfedge_handle, Intersection_type>       Halfedges_hash_w_type;
  typedef Unique_hash_map<Face_handle, bool>                        Faces_hash;

  typedef Unique_hash_map<Vertex_handle, Vertex_handle>             Vertices_map;
  typedef Unique_hash_map<Halfedge_handle, Halfedge_handle>         Halfedges_map;
  typedef Unique_hash_map<Face_handle, Face_handle>                 Faces_map;

  typedef Unique_hash_map<Vertex_handle, Halfedge_handle>           Vertices_to_edges_map;
  
  typedef std::pair<Curve_2, Intersection_type>                     Intersection_curve;
  typedef std::list<Object>                                         Intersections_list;

  // this is used in the resolve edge process
  typedef Triple<Point_2, bool, bool>                               Point_2_with_info;
  struct Points_compare
  {
    protected:
      Traits* p_traits;
    public:
      Points_compare(Traits& tr) : p_traits(&tr)
      {}

      bool operator() (const Point_2_with_info& p1,
                       const Point_2_with_info& p2) const
      {
        Comparison_result res =
          p_traits->compare_xy_2_object()(p1.first, p2.first);
        if (res == SMALLER)
          return true;
        if (res == LARGER)
          return false;
        return (p1.second == true || p2.third == true);
      }
  };
  
public:
  // c'tor
  Envelope_element_visitor_3()
  {
    // Allocate the traits.
    traits = new Traits;
    own_traits = true;

    reset_statistics();
  }

  Envelope_element_visitor_3(Traits* tr)
  {
    // Set the traits.
    traits = tr;
    own_traits = false;

    reset_statistics();
  }

  // virtual destructor.
  virtual ~Envelope_element_visitor_3()
  {
    // Free the traits object, if necessary.
    if (own_traits)
      delete traits;
  }

  // get a face with 2 surfaces defined over it, and compute the arrangement of the
  // envelope of these surfaces over the face
  void resolve(Face_handle face, Minimization_diagram_2& result)
  {
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "in general resolve face" << std::endl;                                  
    #endif

    CGAL_assertion(face->get_aux_is_set(0));
    CGAL_assertion(face->get_aux_is_set(1));
    
    intersection_timer.start();
    // we are interested with the envelope's shape over the current face,
    // so we only need to check the first surface from each group, since
    // all the surfaces in a group overlap over the current face.
    Xy_monotone_surface_3 surf1 = get_aux_surface(face, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(face, 1);
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "compared surfaces are:" << surf1 << std::endl << surf2 << std::endl;
    #endif

    // find the projected intersections of the surfaces. if none - we have a simple case:
    // need only resolve non-intersecting and return
    std::list<Object> inter_objs;
    get_projected_intersections(surf1, surf2, std::back_inserter(inter_objs));
    intersection_timer.stop();

    if (inter_objs.size() == 0)
    {
      minimal_face_timer.start();
      // here for resolve we can compare the surfaces over the edges only (no need for left/right versions)
      Comparison_result cur_res = resolve_minimal_face(face);
      copy_data_by_comparison_result(face, face, cur_res);
      // check face boundary for "data from face" features
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        copy_data_to_face_boundary(face);
      #endif
      minimal_face_timer.stop();
      return;
    }
    
    copied_arr_timer.start();
    // we insert all projected intersections into a temporary arrangement,
    // with only the current face's curves, to find the arrangement of the lower envelope
    // of the 2 surfaces over the current face
    Minimization_diagram_2 copied_face_arr(traits);

    // here we maintain a mapping between edges in the copied arrangement and
    // their original generating edge from result
    Halfedges_map map_copied_to_orig_halfedges;
    // here we maintain a mapping between vertices in the copied arrangement and
    // their original generating vertex from result
    Vertices_map  map_copied_to_orig_vertices;
    // here we maintain a mapping between faces in the copied arrangement and
    // their corresponding face from result
    Faces_map     map_copied_to_orig_faces;

    // now, insert the face's boundary into the temporary minimization diagram
    // the face is assumed to have outer boundary, and may also have holes,
    // and isolated vertices
    // we need to keep track of the original halfedges in the inserted halfedges
    // we also need to have the face handle of the copied face in copied_face_arr
    bool fakes_exist = false;
    Face_handle copied_face = copy_face(face, result, copied_face_arr,
                                        map_copied_to_orig_halfedges,
                                        map_copied_to_orig_vertices,
                                        fakes_exist);
    CGAL_assertion(is_valid(copied_face_arr));
    map_copied_to_orig_faces[copied_face] = face;
    copied_arr_timer.stop();
    
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of face's edges: " << copied_face_arr.number_of_edges() << std::endl;
    #endif
    
    // insert the projected intersections into the temporary minimization diagram
    zone_timer.start();
    Point_2 point;
    Intersection_curve curve;
    Object cur_obj;
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "resolve face: need to deal with " << inter_objs.size()
                << " intersection objects" << std::endl;
    #endif

    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      // print all edges of result
      std::cout << "from general resolve face: all edges before insert " << std::endl;
      for (Halfedge_iterator hhit = result.halfedges_begin();
           hhit != result.halfedges_end(); ++hhit, ++hhit)
        std::cout << hhit->curve() << std::endl;
    #endif
    
    // we use our zone visitor, which only inserts into the arrangement the
    // points and curves which are inside the copied face
    // it updates the result arrangement at the same time (action after action
    // using observer to the copied arrangement and accessor to result)
    // the visitor is responsible for updating:
    // 1. the collection of special edges. (these are (parts of) edges of the

    //    original face's boundary that overlap with projected intersections
    Halfedges_list result_special_edges;
    // 2. the collection of newly added edges, each with the type of the
    //    projected intersection that created it.
    Halfedges_w_type_list result_new_edges;
    // 3. the collection of faces that form the face before any insertion
    Faces_list     result_face_parts;
    // 4. the collection of special vertices, which contains:
    //    - new vertices that were created inside the original face
    //      (both isolated and not isolated)
    //    - new vertices created by a split of a boundary edge which has
    //      the property "data from face"
    //    - original vertices of the boundary that consolidate with projected
    //      intersections, and have common aux data with the face
    //    all these vertices should have their data set as "EQUAL"
    Vertices_list  result_special_vertices;

    New_faces_observer new_faces_obs(result);    
    Copied_face_zone_visitor zone_visitor(result, copied_face_arr,
                                          face,
                                          copied_face,
                                          map_copied_to_orig_halfedges,
                                          map_copied_to_orig_vertices,
                                          map_copied_to_orig_faces,
                                          result_special_edges,
                                          result_new_edges,
                                          result_face_parts,
                                          result_special_vertices,
                                          this);

    Md_point_location pl(copied_face_arr);
    std::list<Object>::iterator inter_objs_it = inter_objs.begin();
    for(; inter_objs_it != inter_objs.end(); ++inter_objs_it)
    {
      cur_obj = *inter_objs_it;
      CGAL_assertion(!cur_obj.is_empty());
      if (assign(point, cur_obj))
      {
        // intersection can be a point when the surfaces only touch each other.
        // we are only interested in the points that are inside the face or on its
        // boundary.
        // we insert the point into the planar map as a vertex, with both surfaces
        // over it.
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "found intersection point: " << point << std::endl;
        #endif
        // should use observer for split_edge
        // if not in a sub-face of "face", shouldn't insert it
        // the above information is available in zone_visitor
        insert_point(copied_face_arr, point, pl, zone_visitor);
      }
      else if (assign(curve, cur_obj))
      {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "found intersection curve: " << curve.first << std::endl;
        #endif
        zone_visitor.set_current_intersection_type(curve.second);
        insert_curve(copied_face_arr, curve.first, pl, zone_visitor);
      }
      else
        CGAL_assertion_msg(false, "wrong projected intersection type");
    }

    zone_visitor.finish();
    zone_timer.stop();

    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of new edges: "
                << result_new_edges.size() << std::endl;
    #endif
 
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of edges on face boundary that overlap projected intersections is "
                << result_special_edges.size() << std::endl;
    #endif
       
    // now determine the envelope data in result over the new faces
    minimal_face_timer.start();

    // first, we try to copy information from incident faces, thru fake edges
    if (fakes_exist)
    {
      typename std::list<Face_handle>::iterator fit;
      for(fit = result_face_parts.begin();
          fit != result_face_parts.end(); ++fit)
      {
        Face_handle new_f = *fit;
        // we didn't set envelope data yet
        CGAL_assertion(!new_f->is_decision_set());
        // try to find a fake edge on the outer boundary
        // (if we have fake edges, we don't have holes)
        Ccb_halfedge_circulator hec = new_f->outer_ccb();
        Ccb_halfedge_circulator hec_begin = hec;
        do {
          Halfedge_handle hh = hec;
          if (hh->get_is_fake() &&
              hh->twin()->face()->is_decision_set())
          {
            Face_handle twin_f = hh->twin()->face();
            new_f->set_decision(twin_f->get_decision());
	    
      	    hh->set_decision(twin_f->get_decision());
      	    hh->twin()->set_decision(hh->get_decision());
          }
          ++hec;
        } while (hec != hec_begin);
      }
    }
    
    // in order to use resolve_minimal_face with intersection halfedge, we go over
    // the new edges, and set data over their faces
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of new edges: " << result_new_edges.size() << std::endl;
    #endif
    typename Halfedges_w_type_list::iterator new_edge_it;
    for(new_edge_it = result_new_edges.begin();
        new_edge_it != result_new_edges.end(); ++new_edge_it)
    {
      Halfedge_handle new_he = (*new_edge_it).first;
      Halfedge_handle new_he_twin = new_he->twin();
      Intersection_type itype = (*new_edge_it).second;

      // set sources of the new edge
      new_he->set_aux_source(0, face->get_aux_source(0));
      new_he->set_aux_source(1, face->get_aux_source(1));
      new_he_twin->set_aux_source(0, face->get_aux_source(0));
      new_he_twin->set_aux_source(1, face->get_aux_source(1));

      // set data on new edges
      new_he->set_decision(EQUAL);
      new_he_twin->set_decision(EQUAL);
      
      // set data on the faces
      // could be that the data is set for f2, and can use itype to conclude
      // to f1, not only the opposite
      Face_handle f1 = new_he->face(), f2 = new_he_twin->face();
      Comparison_result res;
      if (!f1->is_decision_set() && !f2->is_decision_set())
      {
        res = resolve_minimal_face(f1, &new_he);
        copy_data_by_comparison_result(face, f1, res);
      }

      // now at least one of the faces f1,f2 has its decision set.      

      // if the other face doesn't have its data, we resolve it using
      // the former result and the intersection type (if exists)
      if (!f2->is_decision_set())
      {
        #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
          if (itype != UNKNOWN)
          {
            res = convert_decision_to_comparison_result(f1->get_decision());
            res = resolve_by_intersection_type(res, itype);
            CGAL_expensive_assertion_code(
              Comparison_result tmp_res = resolve_minimal_face(f2, &new_he_twin);
            );
            CGAL_expensive_assertion(tmp_res == res);
          }
          else
            res = resolve_minimal_face(f2, &new_he_twin);
        #else
          res = resolve_minimal_face(f2, &new_he_twin);
        #endif
        copy_data_by_comparison_result(face, f2, res);
      }
      if (!f1->is_decision_set())
      {
        #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
          if (itype != UNKNOWN)
          {
            res = convert_decision_to_comparison_result(f2->get_decision());
            res = resolve_by_intersection_type(res, itype);
            CGAL_expensive_assertion_code(
              Comparison_result tmp_res = resolve_minimal_face(f1, &new_he);
            );
            CGAL_expensive_assertion(tmp_res == res);
          }
          else
            res = resolve_minimal_face(f1, &new_he);
        #else
          res = resolve_minimal_face(f1, &new_he);
        #endif
        copy_data_by_comparison_result(face, f1, res);
      }

    }
    
    // we also need to check the faces incident to the halfedges in special_edges
    // since the envelope data over them should be computed using compare_left/right versions
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of special edges: " << result_special_edges.size()
                << std::endl;
    #endif
    Halfedges_list_iterator special_edge_it;
    for(special_edge_it = result_special_edges.begin();
        special_edge_it != result_special_edges.end(); ++special_edge_it)
    {
      // we assume that the halfedge given points to the correct face (which is inside the original face)
      Halfedge_handle special_he = *special_edge_it;
      Face_handle f = special_he->face();
      if (!f->is_decision_set())
      {
        Comparison_result res = resolve_minimal_face(f, &special_he);
        copy_data_by_comparison_result(face, f, res);      
      }

      // take care for the edge, if necessary
      if (!special_he->is_decision_set() &&
          can_copy_decision_from_face_to_edge(special_he))
      {
        if (!special_he->get_aux_is_set(0) || !special_he->get_aux_is_set(1))
        {
          // this can only happen when the edge is fake, since the edge is on 
		  // the face's boundary
          CGAL_assertion(special_he->get_is_fake());
          special_he->set_aux_source(0, face->get_aux_source(0));
          special_he->set_aux_source(1, face->get_aux_source(1));
          special_he->twin()->set_aux_source(0, face->get_aux_source(0));
          special_he->twin()->set_aux_source(1, face->get_aux_source(1));
	      }
      	if (special_he->get_is_fake())
      	{
      	  // this edge is not fake anymore, as it coincides with a projected
      	  // intersection
      	  special_he->set_is_fake(false);
      	  special_he->twin()->set_is_fake(false);
        }
        special_he->set_decision(EQUAL);
        special_he->twin()->set_decision(EQUAL);
      }
    }

    // update data on special vertices
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "number of special vertices: " << result_special_vertices.size() << std::endl;
    #endif
    Vertices_list_iterator special_vertex_it;
    for(special_vertex_it = result_special_vertices.begin();
        special_vertex_it != result_special_vertices.end(); ++special_vertex_it)
    {
      Vertex_handle special_v = *special_vertex_it;
      if (!special_v->is_decision_set())
      {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "special vertex " << special_v->point() << std::endl;
        #endif
      
        if (special_v->get_aux_is_set(0) && special_v->get_aux_is_set(1))
          set_data_by_comparison_result(special_v, EQUAL);
        else
		  // this is a new vertex inside the face, so we need to update its
		  // aux source information from face also (done in method)
          copy_all_data_to_vertex(face, special_v);
      } else
        CGAL_assertion(special_v->get_aux_is_set(0) && special_v->get_aux_is_set(1));
     }
    
    // assert all new faces got data set, if not, then maybe no curve cuts the face,
    // and should use regular resolve_minimal_face
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "check all face parts: " << result_face_parts.size() << std::endl;
    #endif
    typename std::list<Face_handle>::iterator new_face_it;
    for(new_face_it = result_face_parts.begin();
        new_face_it != result_face_parts.end(); ++new_face_it)
    {
      Face_handle new_face = *new_face_it;
      if (!new_face->is_decision_set())
      {
        Comparison_result res = resolve_minimal_face(new_face);
        copy_data_by_comparison_result(face, new_face, res);
      }

      // check face boundary for "data from face" features
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        copy_data_to_face_boundary(new_face);
      #endif
    }

    minimal_face_timer.stop();
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "finish resolve face " << std::endl;
    #endif

  }    

  // get an edge with 2 surfaces defined over it, and split it to get the shape
  // of the envelope of these surfaces over the edge


  void resolve(Halfedge_handle edge, Minimization_diagram_2& result)
  {
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "in resolve edge" << std::endl;
    #endif
    Xy_monotone_surface_3 surf1 = get_aux_surface(edge, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(edge, 1);
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "resolve edge: compared surfaces are:" << surf1 << std::endl << surf2 << std::endl;
    #endif

    // find the projected intersections
    std::list<Object> inter_objs;
    edge_intersection_timer.start();
    get_projected_intersections(surf1, surf2, std::back_inserter(inter_objs));
    edge_intersection_timer.stop();

    if (inter_objs.size() == 0)
    {
      resolve_minimal_edge(edge, edge);
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        copy_data_to_edge_endpoints(edge);
      #endif
      return;
    }

    edge_2d_inter_timer.start();
    const X_monotone_curve_2& original_cv = edge->curve();
    const Point_2& original_left = traits->construct_min_vertex_2_object()(original_cv);
    const Point_2& original_right = traits->construct_max_vertex_2_object()(original_cv);

    // we want to work on the halfedge going from left to right
    if (original_left != edge->source()->point())
      edge = edge->twin();
      
    Vertex_handle original_src = edge->source();
    Vertex_handle original_trg = edge->target();
    
    // we should have a list of points where we should split the edge's curve
    // we then will sort the list, and split the curve

    // we should pay a special attension for overlaps, since we can get special
    // edges

    // we associate with every point 2 flags:
    // 1. is the point a left endpoint of an overlapping segment
    // 2. is the point a right endpoint of an overlapping segment
    typedef std::vector<Point_2_with_info>      Points_vec;
    Points_vec                                  split_points;

    Point_2 point;
    Intersection_curve icurve;
    Object cur_obj;

    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "resolve edge: need to deal with " << inter_objs.size()
                << " intersection objects" << std::endl;
    #endif

    std::list<Object>::iterator inter_objs_it = inter_objs.begin();
    for(; inter_objs_it != inter_objs.end(); ++inter_objs_it)
    {
      cur_obj = *inter_objs_it; 
      CGAL_assertion(!cur_obj.is_empty());
      if (assign(point, cur_obj))
      {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "found intersection point: " << point << std::endl;
        #endif

        // if the point is on the curve, should add it the the split points
        // list, otherwise, it is irrelevant and should be ignored
        if (is_point_on_curve(point, original_cv, original_left, original_right))
          split_points.push_back(Point_2_with_info(point, false, false));
      }
      else if (assign(icurve, cur_obj))
      {
        Curve_2 curve = icurve.first;
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "found intersection curve: " << curve << std::endl;
        #endif

        // find the intersection points and overlapping segments with the
        // original curve and insert them to the list of split points

        // first, get x_monotone parts
        std::list<Object>                     x_objects;
        std::list<Object>::const_iterator     obj_it;
        const X_monotone_curve_2             *x_curve;
        const Point_2                        *iso_p;

        traits->make_x_monotone_2_object()(curve,
                                           std::back_inserter(x_objects));

        for (obj_it = x_objects.begin(); obj_it != x_objects.end(); ++obj_it)
        {
          x_curve = object_cast<X_monotone_curve_2> (&(*obj_it));
          if (x_curve != NULL)

          {
            // intersect the x-monotone curve with the edge's curve
            typedef std::pair<Point_2, unsigned int> Intersect_point_2;
            std::list<Object> intersections_list;
            const Intersect_point_2  *ip;
            const X_monotone_curve_2 *icv;
            
            traits->intersect_2_object()(*x_curve, original_cv,
                                         std::back_inserter(intersections_list));

            std::list<Object>::iterator inter_it = intersections_list.begin();
            for(; inter_it != intersections_list.end(); ++inter_it)
            {
              ip = object_cast<Intersect_point_2> (&(*inter_it));
              if (ip != NULL)
              {
                split_points.push_back(Point_2_with_info(ip->first, false, false));
              }
              else
              {
                icv = object_cast<X_monotone_curve_2> (&(*inter_it));
                CGAL_assertion (icv != NULL);

                split_points.push_back(Point_2_with_info(
                                           traits->construct_min_vertex_2_object()(*icv),
                                           true, false));
                split_points.push_back(Point_2_with_info(
                                           traits->construct_max_vertex_2_object()(*icv),
                                           false, true));
              }
            }            
          }
          else
          {
            iso_p = object_cast<Point_2> (&(*obj_it));
            CGAL_assertion (iso_p != NULL);
            // if the point is on the curve, should add it the the split points
            // list, otherwise, it is irrelevant and should be ignored
            if (is_point_on_curve(*iso_p, original_cv, original_left, original_right))
              split_points.push_back(Point_2_with_info(*iso_p, false, false));
          }
        }        
      }
      else
        CGAL_assertion_msg(false, "wrong projected intersection type");
    }
    edge_2d_inter_timer.stop();
    
    // if there aren't any split points, we can finish
    if (split_points.size() == 0)
    {
      resolve_minimal_edge(edge, edge);
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        copy_data_to_edge_endpoints(edge);
      #endif
      return;
    }

    edge_split_timer.start();
    
    // sort the split points from left to right
    // and split the original edge in these points
    Points_compare comp(*traits);
    std::sort(split_points.begin(), split_points.end(), comp);

    // find if vertical surfaces are involved here, since if not, we can save
    // calls to traits methods (we know that over projected intersection the
    // surfaces are equal on the envelope, which is not neccessarily true if
    // vertical surfaces are involved)
    bool are_verticals_involved = false;
//    if (traits->is_vertical_3_object()(surf1) ||
//        traits->is_vertical_3_object()(surf2))
//      are_verticals_involved = true;    
    
    // check if source is a special vertex (i.e. also a projected intersection)
    // by checking the first point in the list
    bool source_is_special = false;
    CGAL_assertion(split_points.size() >= 1);
    if (split_points[0].first == original_src->point())
      source_is_special = true;
    
    // check if target is a special vertex, by checking the last point in the list
    bool target_is_special = false;
    if (split_points[split_points.size()-1].first == original_trg->point())
      target_is_special = true;

    // if overlaps > 0 it will indicate that we are inside an overlapping segment
    // meaning, we have a special edge
    int overlaps = 0;    

    // remember the envelope decision over the first & last parts, to
    // be able to copy it to the original endpoints
    // TODO: the initial value is only needed to shut up the compiler
    // TODO: is this realy needed, or we can use the decision made on "edge"
    // (is "edge" always the first part? )
    Comparison_result first_part_res = EQUAL;
    
    // cur_part is the part of the original edge that might be split
    Halfedge_handle cur_part = edge;

    for(unsigned int i=0; i<split_points.size(); ++i)
    {
      Point_2_with_info cur_p = split_points[i];
      // if we get to the target vertex, we end the loop, since no more splits

      // are needed
      if (cur_p.first == original_trg->point())
        break;
        
      Vertex_handle cur_src_vertex = cur_part->source();

      // check that the current split point is not already a vertex
      if (cur_p.first != cur_src_vertex->point())
      {
        // split the edge in this point
        X_monotone_curve_2 a,b;
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "before edge_split, curve=" << cur_part->curve() << std::endl <<
                       " point= " << cur_p.first << std::endl;
        #endif
        traits->split_2_object()(cur_part->curve(), cur_p.first, a, b);
        // todo: can we use the internal split?
        Halfedge_handle split_he = result.split_edge(cur_part, a, b);
        // split always returns the halfedge with source = cur_part.source
        CGAL_assertion(split_he->source() == cur_src_vertex);

        // the new vertex is split_he->target(), we set envelope data on it
        if (!are_verticals_involved)
          copy_all_data_to_vertex(edge, split_he->target());
        else
          //resolve(split_he->target());
          deal_with_new_vertex(edge, split_he->target());

        // identify the part of the split edge that we are finished with
        // (this is the one with cur_src_vertex),
        // and the part that might be split again
        Halfedge_handle finished_part = split_he;
        cur_part = split_he->next();

        // set the envelope data over the finished part
        // if the finished part is a special edge, and no verticals are involved
        // we can set both aux data on it. otherwise we should use the traits
        // compare method.
        Comparison_result finished_part_res;
        if (overlaps > 0 && !are_verticals_involved)
          finished_part_res = EQUAL;
        else
          finished_part_res = resolve_minimal_edge(edge, finished_part);

        finished_part->set_decision(finished_part_res);
        finished_part->twin()->set_decision(finished_part_res);

        if (finished_part == edge)
          first_part_res = finished_part_res;
      }

      // check the overlaps indications
      if (cur_p.second == true)
        ++overlaps; // we start a new overlapping segment at this point
      if (cur_p.third == true)
        --overlaps; // we end an overlapping segment at this point
              
    }

    // set envelope data on the last part (cur_part)
    // if the last part is a special edge, and no verticals are involved
    // we can set both aux data on it. otherwise we should use the traits
    // compare method.
    Comparison_result cur_part_res;
    if (overlaps > 0 && !are_verticals_involved)
      cur_part_res = EQUAL;
    else
      cur_part_res = resolve_minimal_edge(edge, cur_part);

    cur_part->set_decision(cur_part_res);
    cur_part->twin()->set_decision(cur_part_res);

    if (cur_part == edge)
      first_part_res = cur_part_res;
      
    // if the original source and target have same aux data as the edge
    // we can set envelope data over them also
    // if they are special vertices, we set both aux data. otherwise we copy
    // from the incident edge part.


    // the incident edge part to source should be edge (the first part)
    CGAL_assertion(original_src == edge->source());
    if (!original_src->is_decision_set() &&
        !are_verticals_involved &&
        can_copy_decision_from_edge_to_vertex(edge->twin()))
    {
      if (source_is_special)
        set_data_by_comparison_result(original_src, EQUAL);
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        else
          set_data_by_comparison_result(original_src, first_part_res);
      #endif

    }
    // the incident edge part to target should be cur_part (the last part)
    CGAL_assertion(original_trg == cur_part->target());
    if (!original_trg->is_decision_set() &&
        !are_verticals_involved &&
        can_copy_decision_from_edge_to_vertex(cur_part))
    {
      if (target_is_special)
        set_data_by_comparison_result(original_trg, EQUAL);
      #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
        else
          set_data_by_comparison_result(original_trg, cur_part_res);
      #endif
    }
    edge_split_timer.stop();    

  }
  
  // get a vertex with 2 surfaces defined over it and decide the envelope data
  // on it between them
  void resolve(Vertex_handle vertex)
  {
    // it is enough to compare only one surface from each group (because they
    // all overlap over the vertex), but set all the group
    Xy_monotone_surface_3 surf1 = get_aux_surface(vertex, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(vertex, 1);
    Point_2 point_2 = vertex->point();
    Comparison_result cur_res = compare_distance_to_envelope(point_2,surf1,surf2);
    vertex->set_decision(cur_res);
  }

  /*! Access the traits object (const version). */
  const Traits* get_traits () const
  {
    return (traits);
  }

  /*! Access the traits object (non-const version). */
  Traits* get_traits ()
  {
    return (traits);
  }

  void reset()
  {
    reset_statistics();
  }

  void reset_statistics()
  {
    if (intersection_timer.is_running())
      intersection_timer.stop();
    if (copied_arr_timer.is_running())
      copied_arr_timer.stop();
    if (zone_timer.is_running())
      zone_timer.stop();
    if (minimal_face_timer.is_running())
      minimal_face_timer.stop();

    if (edge_intersection_timer.is_running())
      edge_intersection_timer.stop();
    if (edge_2d_inter_timer.is_running())
      edge_2d_inter_timer.stop();
    if (edge_split_timer.is_running())
      edge_split_timer.stop();

    intersection_timer.reset();
    copied_arr_timer.reset();
    zone_timer.reset();
    minimal_face_timer.reset();

    edge_intersection_timer.reset();
    edge_2d_inter_timer.reset();
    edge_split_timer.reset();
  }
  
  void print_times()
  {
    #ifdef CGAL_BENCH_ENVELOPE_DAC
      std::cout << "resolve face times: " << std::endl;
      std::cout << "intersections: " << intersection_timer.time() << " seconds" << std::endl;
      std::cout << "copied_arr: " << copied_arr_timer.time() << " seconds" << std::endl;
      std::cout << "zone calculation: " << zone_timer.time() << " seconds" << std::endl;
      std::cout << "envelope data: " << minimal_face_timer.time() << " seconds" << std::endl;
      std::cout << std::endl;
      std::cout << "resolve edge times: " << std::endl;
      std::cout << "intersections: " << edge_intersection_timer.time() << " seconds" << std::endl;
      std::cout << "2d intersections: " << edge_2d_inter_timer.time() << " seconds" << std::endl;
      std::cout << "split & compare: " << edge_split_timer.time() << " seconds" << std::endl;

      std::cout << std::endl
          << "determine a feature's shape took: "
          << intersection_timer.time() + copied_arr_timer.time() + zone_timer.time()
          << "seconds " << std::endl
          << "labelling took: " << minimal_face_timer.time() <<std::endl;

    #endif    
  }

protected:
  
  // compute Comparison_result of surfaces over the face, assuming they get 
  // the same answer for all points in face
  // if we get a halfedge, it is assumed to be on the outer boundary of the 
  // face, and its curve is assumed to be a projected intersection curve, 
  // and we compare the surfaces to the left/right of it
  // otherwise we compare the surfaces over an (arbitrary) edge of the face,
  // assuming this is the correct answer for the face since the surfaces are 
  // continous
  // In either case, we try to copy decision from an incident face, is possible
  // before asking the geometric question
  Comparison_result resolve_minimal_face(Face_handle face, 
                                         Halfedge_handle* he = NULL)
  {
    CGAL_precondition(he == NULL || (*he)->face() == face);
    CGAL_assertion(!face->is_unbounded());
    Comparison_result res;

    bool success = false;
    #ifdef CGAL_ENVELOPE_SAVE_COMPARISONS
      success = can_copy_decision_from_boundary_edge(face, res);
    #endif
    
    if (success)
      return res;
      
    Xy_monotone_surface_3 surf1 = get_aux_surface(face, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(face, 1);

    if (he == NULL)
    {
      // compare the surfaces over arbitrary edge
      Ccb_halfedge_circulator hec = face->outer_ccb();
      X_monotone_curve_2 cv = hec->curve();
      res = compare_distance_to_envelope(cv,surf1,surf2);
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in resolve_minimal_face, with no halfedge, got " 
                  << res << std::endl;
      #endif
	
      // check that result is equal on all edges
      CGAL_assertion_code(	
        Ccb_halfedge_circulator hec_begin = hec;    
        ++hec;
        while (hec != hec_begin)
        {
          Comparison_result tmp = 
                        compare_distance_to_envelope(hec->curve(),surf1,surf2);
      )
      CGAL_assertion_msg(tmp == res, 
                         "compare over curve returns non-consistent results");
      CGAL_assertion_code(
          ++hec;

        }
      )
    }
    else
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in resolve_minimal_face, with halfedge " << std::endl;
      #endif
      // compare the surfaces over the halfedge's curve
      X_monotone_curve_2 cv = (*he)->curve();

      CGAL_assertion_code(
        bool same_dir = (traits->construct_min_vertex_2_object()(cv) ==
                         (*he)->source()->point());
        bool left_to_right = ((*he)->direction() == SMALLER);
      );
      CGAL_assertion(same_dir == left_to_right);

      // a face is always to the left of its halfedge
      if ((*he)->direction() == SMALLER)
        res = traits->compare_distance_to_envelope_above_3_object()
                                                   (cv,surf1,surf2);
      else
        res = traits->compare_distance_to_envelope_below_3_object()
                                                   (cv,surf1,surf2);
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "resolve_minimal_face check " 

                  << (same_dir ? "left " : "right ") 
                  << "of curve " << cv << std::endl << "got " << res 
                  << std::endl;
      #endif
    }
    return res;
  }

  // use the Intersection type (Transversal/Tangent) and return the appropriate
  // comparison result of the other side of the intersection curve, 
  // if the first side has result "res"
  Comparison_result resolve_by_intersection_type(Comparison_result res,
                                                 Intersection_type itype)
  {
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "resolve_by_intersection_type called" << std::endl;
    #endif
    
    if (itype == TRANSVERSAL)
    {
      if (res == LARGER)
        return SMALLER;
      else if (res == SMALLER)
        return LARGER;
      else
        return res;
    }
    else
    {
      CGAL_assertion(itype == TANGENT);
      return res;
    }

  }
  
  // find intersections between 2 xy-monotone surfaces
  // use caching for repeating questions of same pair of surfaces
  template <class OutputIterator>
  OutputIterator get_projected_intersections(Xy_monotone_surface_3& s1,
                                             Xy_monotone_surface_3& s2,
                                             OutputIterator o)
  {
    return traits->construct_projected_intersections_2_object()(s1, s2, o);
  }

  // Geometry can be a Point_2 or a X_monotone_curve_2
  template <class Geometry>
  Comparison_result compare_distance_to_envelope(Geometry& g,
                                                 Xy_monotone_surface_3& s1,
                                                 Xy_monotone_surface_3& s2)
  {
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "compare distance on " << g << std::endl;
    #endif
    return traits->compare_distance_to_envelope_3_object()(g, s1, s2);   
  }

  // helper method to get the surfaces we need to work on
  template <class FeatureHandle>
  Xy_monotone_surface_3 get_aux_surface(FeatureHandle fh, unsigned int id)
  {
	  Object o = fh->get_aux_source(id);
    CGAL_assertion(!o.is_empty());
    Xy_monotone_surface_3 data;
    
    Halfedge_handle h;
    Vertex_handle v;
  	Face_handle f;

    // aux source of a face must be a face!
    // aux source of a halfedge can be face or halfedge
    // aux source of a vertex can be face, halfedge or vertex
    // this is why we start with a check for a face, then halfedge
    // and last vertex
  	if (assign(f, o))
  	  data = f->get_data();
  	else if (assign(h, o))
  	  data = h->get_data();
  	else
  	{
      CGAL_assertion_code(bool b =)
      assign(v, o);
      CGAL_assertion(b);
  	  data = v->get_data();
  	}
    return data;
  }
                                  
  bool can_copy_decision_from_face_to_edge(Halfedge_handle h)
  {
    // can copy decision from face to its incident edge if the aux
    // envelopes are continous over the face and edge
    return (h->get_has_equal_aux_data_in_face(0) &&
            h->get_has_equal_aux_data_in_face(1));
  }

  bool can_copy_decision_from_edge_to_vertex(Halfedge_handle h)
  {
    // can copy decision from face to its incident edge if the aux
    // envelopes are continous over the face and edge
    return (h->get_has_equal_aux_data_in_target(0) &&
            h->get_has_equal_aux_data_in_target(1));
  }


/*  template <class OutputIterator>
  OutputIterator find_ccb_unique_edges(Ccb_halfedge_circulator hec,
                                       OutputIterator o)
  {
    // when having "antena" in the ccb, edge might appear twice
    // (with its 2 halfedges) on the boundary
    // we keep here those halfedges that its twin is also on this face's boundary
    // as a stack
    std::deque<Halfedge_handle> antena;
    Ccb_halfedge_circulator hec_begin = hec;
    do {
      Halfedge_handle hh = hec;
      if (hh->twin()->face() == hh->face() &&
          antena.size() > 0 &&
          antena.back() == hh->twin())
      {
        antena.pop_back();
      }
      else
      {
        if (hh->twin()->face() == hh->face())
           antena.push_back(hh);
        *o++ = hh;
      }
      hec++;
    } while(hec != hec_begin);
    CGAL_assertion(antena.size() == 0);
    return o;
  }
*/

//  void print_ccb(Ccb_halfedge_circulator hec)
//  {
//    std::cout << "print ccb: " << std::endl;
//    Ccb_halfedge_circulator hec_begin = hec;
//    do {
//      Halfedge_handle h = hec;
//      std::cout << h->source()->point() << " --> " << h->target()->point() << std::endl;
//      hec++;
//    } while(hec != hec_begin);
//  }

  // check the aux data on the edges & vertices of the boundary of the face,
  // and if it equals the aux data on the face, copy it, to save calculations for
  // these features later
  // also consider isolated vertices
  // "res" is the decision made on the face
  void copy_data_to_face_boundary(Face_handle face)
  {
    Ccb_halfedge_circulator ccb = face->outer_ccb();
    copy_data_to_face_boundary(face, ccb);

    Hole_iterator inner_iter = face->holes_begin();
    for (; inner_iter != face->holes_end(); ++inner_iter)
    {
      ccb = (*inner_iter);
      copy_data_to_face_boundary(face, ccb);
    }

    Isolated_vertex_iterator iso_iter = face->isolated_vertices_begin();
    for (; iso_iter != face->isolated_vertices_end(); ++iso_iter)
    {
      Vertex_handle vh = iso_iter;
      if (!vh->is_decision_set() && has_equal_aux_data_with_face(vh))
        // can copy the data from the face, since we already took care of
        // the vertices of projected intersections
        vh->set_decision(face->get_decision());
    }
  }

  void copy_data_to_face_boundary(Face_handle face,
                                  Ccb_halfedge_circulator hec)
  {
    Ccb_halfedge_circulator hec_begin = hec;
    do {
      Halfedge_handle hh = hec;
      CGAL_assertion(face == hh->face());
      // if it is a vertical decomposition edge, copy data from face
      if (!hh->is_decision_set() && hh->get_is_fake())
      {
        hh->set_decision(face->get_decision());
        hh->twin()->set_decision(face->get_decision());
      }
      else if (!hh->is_decision_set() && can_copy_decision_from_face_to_edge(hh))
      {
        // copy the decision from face to the edge
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "copy from face to edge " << hh->curve() << std::endl;
        #endif
        hh->set_decision(face->get_decision());
        hh->twin()->set_decision(hh->get_decision());
      }
      // TODO: is this correct? shouldn't we split the edge first?
      // I think it is correct, because there is no intersection (of
      // the edges aux sources) over the edge, as if there was such
      // intersection, there would also be intersection between the surfaces
      // over the face, and we know now that there isn't.
      
      // if the first map is continous, but the second isn't (i.e. when we move
      // from the face to the edge, the envelope goes closer), then if the
      // second map wins on the face, it wins on the edge also
      else if (!hh->is_decision_set() &&
               face->get_decision() == SECOND &&
               hh->get_has_equal_aux_data_in_face(0) &&
               !hh->get_has_equal_aux_data_in_face(1))
      {
        hh->set_decision(SECOND);
        hh->twin()->set_decision(SECOND);

      }
      // if the second map is continous, but the first isn't, then if the
      // first map wins on the face, it wins on the edge also
      else if (!hh->is_decision_set() &&
               face->get_decision() == FIRST &&
               !hh->get_has_equal_aux_data_in_face(0) &&
               hh->get_has_equal_aux_data_in_face(1))
      {
        hh->set_decision(FIRST);
        hh->twin()->set_decision(FIRST);
      }

      // conclude to the vertices
      // we check both endpoints, since it can be possible that we cannot
      // conclude from one edge, bt can conclude from the other
      conclude_decision_to_vertex(hh->source(), hh->twin(), face, false);
      conclude_decision_to_vertex(hh->target(), hh, face, true);
                      
      hec++;
    } while(hec != hec_begin); 
  }

  // try to conclude the decision from the halfedge or the face to the vertex
  // the method assumes that the vertex is an endpoint of the edge represented
  // by "hh", which lies on the boundary of "fh"
  // the last bool indicates whether to check if possible to conclude from
  // face to vertex. it is only possible when hh->face == fh
  void conclude_decision_to_vertex(Vertex_handle vh, Halfedge_handle hh,
                                   Face_handle fh, bool try_vertex_face)
  {
    if (vh->is_decision_set())
      return;


    // first, we try to copy decision from edge, then from face
    if (hh->is_decision_set() &&
        can_copy_decision_from_edge_to_vertex(hh))
    {
      vh->set_decision(hh->get_decision());
    }
    // if the first map is continous, but the second isn't (i.e. when we move
    // from the edge to the vertex, the envelope goes closer), then if the
    // second map wins on the edge, it wins on the vertex also
    else if (hh->get_decision() == SECOND &&
             hh->get_has_equal_aux_data_in_target(0) &&
             !hh->get_has_equal_aux_data_in_target(1))
    {
      vh->set_decision(SECOND);
    }
    // if the second map is continous, but the first isn't, then if the
    // first map wins on the edge, it wins on the vertex also
    else if (hh->get_decision() == FIRST &&
             !hh->get_has_equal_aux_data_in_target(0) &&
             hh->get_has_equal_aux_data_in_target(1))
    {
      vh->set_decision(FIRST);
    }
    // check if we can copy from the face
    // todo: what if has_equal has 3 possible values? (and projected intersection
    // vertices have unknown flags)
    else if (try_vertex_face)
	{
      CGAL_assertion(has_equal_aux_data_in_target_and_face(hh) == 
	                 has_equal_aux_data(vh, fh));
      if (has_equal_aux_data_in_target_and_face(hh))
	  {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
  	      //std::cout << "check edge->vertex " << hh->curve() << " --> " << vh->point() << std::endl;
          std::cout << "copy from face to vertex " << vh->point() << std::endl;
        #endif

        // can copy the data from the face, since we already took care of
        // the vertices of projected intersections
        vh->set_decision(fh->get_decision());
	  }
	}
  }
  
  // todo: this is for checking
  template <class InputIterator>
  bool has_equal_data(const InputIterator & begin1,
                      const InputIterator & end1,
                      const InputIterator & begin2,
                      const InputIterator & end2)
  {
    // insert the input data objects into a set
    std::set<Xy_monotone_surface_3> first(begin1, end1);
    std::set<Xy_monotone_surface_3> second(begin2, end2);
    std::list<Xy_monotone_surface_3> intersection;
    std::set_intersection(first.begin(), first.end(),

                          second.begin(), second.end(),
                          std::back_inserter(intersection));

    return (intersection.size() > 0);
  }
  // todo: this is for checking
  template <class FeatureHandle>
  void get_aux_data_iterators(unsigned int id, FeatureHandle fh,
                              Envelope_data_iterator& begin,
                              Envelope_data_iterator& end)
  {
    Halfedge_handle h;
    Vertex_handle v;
  	Face_handle f;

    Object o = fh->get_aux_source(id);
    CGAL_assertion(!o.is_empty());

    if (assign(v, o))
    {
      begin = v->begin_data();
      end = v->end_data();
    }
    else if (assign(h, o))
    {
      begin = h->begin_data();
      end = h->end_data();
    }
    else
   	{
   	  CGAL_assertion(assign(f, o));
      assign(f, o);
      begin = f->begin_data();
      end = f->end_data();
   	}
  }

  // todo: this is for checking
  template <class FeatureHandle1, class FeatureHandle2>
  bool has_equal_aux_data(unsigned int id, FeatureHandle1 fh1, FeatureHandle2 fh2)
  {
    Envelope_data_iterator begin1, end1, begin2, end2;
    get_aux_data_iterators(id, fh1, begin1, end1);
    get_aux_data_iterators(id, fh2, begin2, end2);
	  bool has_eq = has_equal_data(begin1, end1, begin2, end2);
	  return has_eq;
  }

  // todo: this is for checking
  template <class FeatureHandle1, class FeatureHandle2>
  bool has_equal_aux_data(FeatureHandle1 fh1, FeatureHandle2 fh2)
  {
	  return (has_equal_aux_data(0, fh1, fh2) &&
		        has_equal_aux_data(1, fh1, fh2));
  }


//  // check if we can copy the decision made on an incident face to the current
//  // face (by using the edge between the faces)
//  // this is an old version for saving comparisons over a face, and should be replaced
//  // with can_copy_decision_from_boundary
//  bool can_copy_decision_from_incident_face(Face_handle face, Comparison_result& res)
//  {
//    bool result = false;
//    Ccb_halfedge_circulator hec = face->outer_ccb();
//    Ccb_halfedge_circulator hec_begin = hec;
//    do {
//      Halfedge_handle hh = hec;
//      // check the incident face
//      if (hh->twin()->face()->is_decision_set() &&
//          can_copy_decision_from_face_to_edge(hh->twin()) &&
//          can_copy_decision_from_face_to_edge(hh) &&
//          hh->is_decision_set() &&
//          hh->get_decision() != BOTH)
//      {
//        res = convert_decision_to_comparison_result(hh->get_decision());
//        result = true;
//      }
//
//      hec++;
//    } while(hec != hec_begin && !result);
//    return result;
//  }

  // check if we can copy the decision made on a boundary edge to the face
  // if so, res will contain this decision's comparison result
  bool can_copy_decision_from_boundary_edge(Face_handle face, Comparison_result& res)
  {
    bool result = false;
    // check outer boundary
    Ccb_halfedge_circulator hec = face->outer_ccb();
    Ccb_halfedge_circulator hec_begin = hec;
    do {
      Halfedge_handle hh = hec;
      if (can_copy_decision_from_face_to_edge(hh) &&

          hh->is_decision_set() &&
          hh->get_decision() != BOTH)
      {
        res = convert_decision_to_comparison_result(hh->get_decision());
        result = true;
      }
      // if the first map is continous, but the second isn't (i.e. when we move
      // from the edge to the face, the envelope goes farther), then if the
      // first map wins on the edge, it wins on the face also
      else if (hh->is_decision_set() &&
               hh->get_decision() == FIRST &&
               hh->get_has_equal_aux_data_in_face(0) &&
               !hh->get_has_equal_aux_data_in_face(1))
      {
        res = convert_decision_to_comparison_result(FIRST);
        result = true;
      } 
      // if the second map is continous, but the first isn't, then if the
      // second map wins on the edge, it wins on the face also
      else if (hh->is_decision_set() &&
               hh->get_decision() == SECOND &&
               !hh->get_has_equal_aux_data_in_face(0) &&
               hh->get_has_equal_aux_data_in_face(1))
      {
        res = convert_decision_to_comparison_result(SECOND);
        result = true;
      }           
      hec++;
    } while(hec != hec_begin && !result);
    if (result) return true;
    // check inner boundaries
    Hole_iterator hole_iter = face->holes_begin();
    for (; hole_iter != face->holes_end(); ++hole_iter)
    {
      hec = (*hole_iter);
      hec_begin = hec;

      do {
        Halfedge_handle hh = hec;
        if (can_copy_decision_from_face_to_edge(hh) &&
            hh->is_decision_set() &&
            hh->get_decision() != BOTH)
        {
          res = convert_decision_to_comparison_result(hh->get_decision());
          result = true;
        }
        // if the first map is continous, but the second isn't (i.e. when we move
        // from the edge to the face, the envelope goes farther), then if the
        // first map wins on the edge, it wins on the face also
        else if (hh->is_decision_set() &&
                 hh->get_decision() == FIRST &&
                 hh->get_has_equal_aux_data_in_face(0) &&
                 !hh->get_has_equal_aux_data_in_face(1))
        {
          res = convert_decision_to_comparison_result(FIRST);

          result = true;
        }
        // if the second map is continous, but the first isn't, then if the
        // second map wins on the edge, it wins on the face also
        else if (hh->is_decision_set() &&
                 hh->get_decision() == SECOND &&
                 !hh->get_has_equal_aux_data_in_face(0) &&
                 hh->get_has_equal_aux_data_in_face(1))
        {
          res = convert_decision_to_comparison_result(SECOND);
          result = true;
        }
  
        hec++;
      } while(hec != hec_begin && !result);
      if (result) return true;
    }

    return result;
  }

  Comparison_result convert_decision_to_comparison_result(CGAL::Dac_decision d)
  {
    if (d == FIRST)
      return SMALLER;
    else if (d == SECOND)
      return LARGER;
    else
      return EQUAL;
  }
  
  bool has_equal_aux_data_with_face(Vertex_handle v)
  {
    CGAL_assertion(v->is_isolated());
    return (v->get_has_equal_aux_data_in_face(0) &&
            v->get_has_equal_aux_data_in_face(1));
  }

  bool has_equal_aux_data_in_target_and_face(Halfedge_handle h)
  {
    return (h->get_has_equal_aux_data_in_target_and_face(0) &&
            h->get_has_equal_aux_data_in_target_and_face(1));
  }

  // check the aux data on the endpoint vertices of the edge
  // and if it equals the aux data on the edge, copy it, to save calculations for
  // these features later
  void copy_data_to_edge_endpoints(Halfedge_handle edge)
  {
    // take care for source
    if (!edge->source()->is_decision_set() &&
        can_copy_decision_from_edge_to_vertex(edge->twin()))
      // can copy the data from the edge, since we already took care of
      // the vertices of projected intersections
      edge->source()->set_decision(edge->get_decision());
    // if the first map is continous, but the second isn't (i.e. when we move
    // from the edge to the vertex, the envelope goes closer), then if the
    // second map wins on the edge, it wins on the vertex also
    else if (edge->get_decision() == SECOND &&
             edge->twin()->get_has_equal_aux_data_in_target(0) &&
             !edge->twin()->get_has_equal_aux_data_in_target(1))
    {
      edge->source()->set_decision(SECOND);
    }
    // if the second map is continous, but the first isn't, then if the
    // first map wins on the edge, it wins on the vertex also
    else if (edge->get_decision() == FIRST &&
             !edge->twin()->get_has_equal_aux_data_in_target(0) &&
             edge->twin()->get_has_equal_aux_data_in_target(1))
    {
      edge->source()->set_decision(FIRST);
    }

    // take care for target
    if (!edge->target()->is_decision_set() &&
        can_copy_decision_from_edge_to_vertex(edge))
      // can copy the data from the edge, since we already took care of
      // the vertices of projected intersections
      edge->target()->set_decision(edge->get_decision());
    // if the first map is continous, but the second isn't (i.e. when we move
    // from the edge to the vertex, the envelope goes closer), then if the
    // second map wins on the edge, it wins on the vertex also
    else if (edge->get_decision() == SECOND &&
             edge->get_has_equal_aux_data_in_target(0) &&
             !edge->get_has_equal_aux_data_in_target(1))
    {
      edge->target()->set_decision(SECOND);
    }
    // if the second map is continous, but the first isn't, then if the
    // first map wins on the edge, it wins on the vertex also
    else if (edge->get_decision() == FIRST &&
             !edge->get_has_equal_aux_data_in_target(0) &&
             edge->get_has_equal_aux_data_in_target(1))
    {
      edge->target()->set_decision(FIRST);
    }
  }

  
  // copy the halfedges of a ccb (in from) to the md "to" inside the face inside_face
  void copy_ccb(Ccb_halfedge_circulator hec, // the circulator to insert
                Minimization_diagram_2 &from,// the original arrangement
                Face_handle inside_face,     // the face in which we insert it
                Minimization_diagram_2 &to,  // the arrangement to which we insert

                Halfedges_map& map_copied_to_orig_halfedges,
                Vertices_map&  map_copied_to_orig_vertices,
                Halfedges_map& map_orig_to_copied_halfedges,
                Vertices_map&  map_orig_to_copied_vertices,
                bool is_outer_ccb, // do we copy an outer (or inner) ccb
                bool& fakes_exist) // this bool is assumed to be initialized already
  {
    Md_accessor to_accessor(to);
    // count the number of faces that are closed by this ccb
    // (it can be more than 1 in degenerate cases, when closed area hangs
    // on a boundary vertex)
    int n_faces_closed = 0;
    
    Ccb_halfedge_circulator hec_begin = hec;
    bool first_he = true;
    Halfedge_handle copied_prev_he;
    do {
      Halfedge_handle hh = hec;

      // update fakes_exist
      if (hh->get_is_fake()) fakes_exist = true;
      
      if (hh->twin()->face() == hh->face() &&
          map_orig_to_copied_halfedges.is_defined(hh))
      {
        // this can happen in the case of antennas, when we get to the same
        // antena halfedge from the other side
        copied_prev_he = map_orig_to_copied_halfedges[hh];
      }
      else
      {
        X_monotone_curve_2 current_cv = hh->curve();
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
	//std::cout << "original halfedge: " << std::endl;
	//std::cout << hh->source()->point() << " --> " << hh->target()->point()
	//          << std::endl;
        #endif
        if (first_he)
        {             
          first_he = false;
          // create the 2 vertices and connect them with the edge
          // copied_prev_he should be directed from copied_source to copied_target
          Vertex_handle copied_source = to_accessor.create_vertex(hh->source()->point());
          Vertex_handle copied_target = to_accessor.create_vertex(hh->target()->point());
          copied_prev_he = to_accessor.insert_in_face_interior_ex(current_cv,
                                                                  inside_face,
                                                                  copied_source,
                                                                  copied_target,
                                                                  hh->direction());
                                                                          
          map_copied_to_orig_halfedges[copied_prev_he] = hh;
          map_orig_to_copied_halfedges[hh] = copied_prev_he;
          map_copied_to_orig_halfedges[copied_prev_he->twin()] = hh->twin();
          map_orig_to_copied_halfedges[hh->twin()] = copied_prev_he->twin();

          map_copied_to_orig_vertices[copied_prev_he->source()] = hh->source();
          map_orig_to_copied_vertices[hh->source()] = copied_prev_he->source();
          map_copied_to_orig_vertices[copied_prev_he->target()] = hh->target();
          map_orig_to_copied_vertices[hh->target()] = copied_prev_he->target();
        }
        else
        {
          CGAL_assertion(map_copied_to_orig_halfedges[copied_prev_he]->target() == hh->source());

          // insert from vertex: prev_he->target()
          // should check if hh->target is already a vertex in the copied face
          // in which case we should use insert at vertices
          bool use_2_vertices = false;
          Vertex_handle copied_v2;
          if (map_orig_to_copied_vertices.is_defined(hh->target()))
          {
            use_2_vertices = true;
            copied_v2 = map_orig_to_copied_vertices[hh->target()];
          }

          Halfedge_handle copied_new_he;
          if (!use_2_vertices)
          {
            // create vertex for the new target, and insert the new edge
            Vertex_handle copied_target = to_accessor.create_vertex(hh->target()->point());
            copied_new_he = to_accessor.insert_from_vertex_ex(current_cv,
                                                              copied_prev_he,
                                                              copied_target,
                                                              hh->direction());

            
            // the target of copied_new_he is the new vertex, so it is directed
            // the same way as hh in "from"

            // update the vertices maps:
            map_copied_to_orig_vertices[copied_new_he->target()] = hh->target();
            map_orig_to_copied_vertices[hh->target()] = copied_new_he->target();
          }
          else
          {
            #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
              std::cout << "use 2 vertices: " << copied_prev_he->target()->point()
                        << " and " << copied_v2->point()
                        << std::endl;
            #endif
            ++n_faces_closed;
            
            // in order to insert the new edge we should determine the prev
            // halfedge of copied_v2 - this is done be going backwards on the
            // ccb (in the copied arrangement) until finding the first halfedge
            // with target copied_v2
            // (note that going on twin()->next() is problematic in case that
            // the inner boundary we traverse is made of several faces)
            Halfedge_handle copied_prev_v2 = copied_prev_he;
            while(copied_prev_v2->source() != copied_v2)
              copied_prev_v2 = copied_prev_v2->prev();
            copied_prev_v2 = copied_prev_v2->twin();

            CGAL_assertion_code(
              Halfedge_handle tmp =
                  to_accessor.locate_around_vertex(copied_v2, current_cv);
            );
            CGAL_assertion(tmp == copied_prev_v2);

            bool new_face;
            if (is_outer_ccb)
            {
              // if it is the first face created, and the last halfedge to
              // insert, this is a regular outer ccb, with no special
              // degeneracies (around the current vertices, at least)
              // so we can use the accessor method
              if (n_faces_closed == 1 &&
                  map_orig_to_copied_halfedges.is_defined(hh->next()))
              {
                copied_new_he = to_accessor.insert_at_vertices_ex

                                                     (current_cv,
                                                      copied_prev_he,
                                                      copied_prev_v2,
                                                      hh->direction(),
                                                      new_face);
                CGAL_assertion(new_face);
              }
              else
              {
                // TODO:can we use accessor method?
                copied_new_he = to.insert_at_vertices(current_cv, copied_prev_he, copied_prev_v2);
              }
              // in order to use the accessor version, we need to identify
              // the order in which to pass the halfedges
              // (we should be careful in cases where more than one face is
              // created by the outer ccb
            }
            else // copy inner boundary
            {
              // should always flip the side of the edge, because the face
              // that we close is never the copied face, even in strane
              // situations like this: (two faces thouch in vertex)
              //     ------         |\  /|
              //     | |\ |         | \/ |
              //     | | \|         | /\ |
              //     ---            |/  \|
              //
              //
              copied_new_he = to_accessor.insert_at_vertices_ex(current_cv,
                                                                copied_prev_v2,
                                                                copied_prev_he,
                                                                hh->twin()->direction(),
                                                                new_face);
              CGAL_assertion(new_face);
              copied_new_he = copied_new_he->twin();
            }

            CGAL_assertion(copied_new_he->target() == copied_v2);
          }
          // update the halfedges maps:
          map_copied_to_orig_halfedges[copied_new_he] = hh;
          map_copied_to_orig_halfedges[copied_new_he->twin()] = hh->twin();
          map_orig_to_copied_halfedges[hh] = copied_new_he;
          map_orig_to_copied_halfedges[hh->twin()] = copied_new_he->twin();

          // update the previous he
          copied_prev_he = copied_new_he;
        }
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
	//std::cout << "copied halfedge: " << std::endl;
	//std::cout << copied_prev_he->source()->point() << " --> " << copied_prev_he->target()->point()
	//          << std::endl;
        #endif
      }
      hec++;
    } while(hec != hec_begin);

  }

  // copy the halfedges of the boundary of face (in from) to the md "to"
  // return a handle to the copied face in "to"
  // precondition: "to" is empty
  Face_handle copy_face(Face_handle face, Minimization_diagram_2& from,
                        Minimization_diagram_2& to,
                        Halfedges_map& map_copied_to_orig_halfedges,
                        Vertices_map&  map_copied_to_orig_vertices,
                        bool& fakes_exist)
  {   
    CGAL_precondition(to.number_of_vertices() == 0);
    CGAL_precondition(!face->is_unbounded());
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "copying the original face - outer boundary" << std::endl;
    #endif

    fakes_exist = false;
    Vertices_map  map_orig_to_copied_vertices;
    Halfedges_map map_orig_to_copied_halfedges;
    
    Face_handle to_uf = to.unbounded_face();


    // first deal with outer boundary
    Ccb_halfedge_circulator hec = face->outer_ccb();
    copy_ccb(hec, from, to_uf, to,
             map_copied_to_orig_halfedges,
             map_copied_to_orig_vertices,
             map_orig_to_copied_halfedges,
             map_orig_to_copied_vertices,
             true,
             fakes_exist);
    CGAL_assertion(is_valid(to));

    // we need to find the copied face
    Hole_iterator to_uf_hi = to_uf->holes_begin();
    Ccb_halfedge_circulator to_uf_hec = (*to_uf_hi);
    CGAL_assertion(to_uf->number_of_holes() == 1);
    Halfedge_handle to_f_he = to_uf_hec->twin();


    // second, deal with inner boundaries
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "copying the original face - inner boundaries" << std::endl;
    #endif
    Hole_iterator hole_iter = face->holes_begin();
    for (; hole_iter != face->holes_end(); ++hole_iter)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "copying hole" << std::endl;
      #endif
      Ccb_halfedge_circulator he = (*hole_iter);
      copy_ccb(he, from, to_f_he->face(), to,

               map_copied_to_orig_halfedges,
               map_copied_to_orig_vertices,
               map_orig_to_copied_halfedges,
               map_orig_to_copied_vertices,
               false,
               fakes_exist);
      CGAL_assertion(is_valid(to));
    }

    // find the face in "to"
    Face_handle copied_face = to_f_he->face();

    // copy the isolated vertices inside the given face, if any
    // and save them in map_copied_to_orig_vertices
    #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
      std::cout << "copying the original face - isolated vertices" << std::endl;
    #endif
    Isolated_vertex_iterator isolated_iter = face->isolated_vertices_begin();
    for(; isolated_iter != face->isolated_vertices_end(); ++isolated_iter)
    {
      Vertex_handle copied_iso = to.insert_in_face_interior(isolated_iter->point(), copied_face);
      map_copied_to_orig_vertices[copied_iso] = isolated_iter;
      map_orig_to_copied_vertices[isolated_iter] = copied_iso;	  
    }    
    return copied_face;
  }
    
  // set envelope data in face "to" according to the comparison result of the
  // aux data of face "from"
  void copy_data_by_comparison_result(Face_handle from, Face_handle to, Comparison_result res)
  {
    CGAL_assertion_msg(from->get_aux_is_set(0), "aux_data(0) is not set");
    CGAL_assertion_msg(from->get_aux_is_set(1), "aux_data(1) is not set");
    to->set_aux_source(0, from->get_aux_source(0));
    to->set_aux_source(1, from->get_aux_source(1));
    to->set_decision(res);
  }

  // set envelope data in vertex "v" according to the comparison result of the
  // aux data of "v"
  void set_data_by_comparison_result(Vertex_handle v, Comparison_result res)
  {
    CGAL_assertion_msg(v->get_aux_is_set(0), "aux_data(0) is not set");
    CGAL_assertion_msg(v->get_aux_is_set(1), "aux_data(1) is not set");
    v->set_decision(res);
  }

  // set envelope data in halfedge "h" according to the comparison result of the
  // aux data of "h"
  void set_data_by_comparison_result(Halfedge_handle h, Comparison_result res)
  {
    CGAL_assertion_msg(h->get_aux_is_set(0), "aux_data(0) is not set");
    CGAL_assertion_msg(h->get_aux_is_set(1), "aux_data(1) is not set");
    h->set_decision(res);
  }

  // set envelope data in vertex "to" according to the union of both
  // aux data of the feature "from"
  // FeatureHabdle should be a Face_handle, Halfedge_handle or 
  // Vertex_handle
  template <class FeatureHabdle>
  void copy_all_data_to_vertex(FeatureHabdle from, Vertex_handle to)
  {
    CGAL_assertion_msg(from->get_aux_is_set(0), "aux_data(0) is not set");
    CGAL_assertion_msg(from->get_aux_is_set(1), "aux_data(1) is not set");
    CGAL_assertion_msg(!to->is_decision_set(), "data is set in new vertex");
    to->set_aux_source(0, from->get_aux_source(0));
    to->set_aux_source(1, from->get_aux_source(1));
    to->set_decision(EQUAL);
  }

  void deal_with_new_vertex(Halfedge_handle orig_he, Vertex_handle new_v)
  {
    Xy_monotone_surface_3 surf1 = get_aux_surface(orig_he, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(orig_he, 1);

    Point_2 p = new_v->point();
    Comparison_result res = compare_distance_to_envelope(p, surf1, surf2);
    new_v->set_aux_source(0, orig_he->get_aux_source(0));
    new_v->set_aux_source(1, orig_he->get_aux_source(1));
    new_v->set_decision(res);
  }
    
  Comparison_result resolve_minimal_edge(Halfedge_handle orig_he, Halfedge_handle new_he)
  {
    // find and set the envelope data on the new edge
    Xy_monotone_surface_3 surf1 = get_aux_surface(orig_he, 0);
    Xy_monotone_surface_3 surf2 = get_aux_surface(orig_he, 1);
    Comparison_result res = compare_distance_to_envelope(new_he->curve(), surf1, surf2);

//    new_he->set_aux_source(0, orig_he->get_aux_source(0));
//    new_he->set_aux_source(1, orig_he->get_aux_source(1));

//    new_he->twin()->set_aux_source(0, orig_he->get_aux_source(0));
//    new_he->twin()->set_aux_source(1, orig_he->get_aux_source(1));

   	// the observer keeps this information when splitting an edge
   	CGAL_assertion(new_he->get_aux_is_set(0) && new_he->get_aux_is_set(1));
   	CGAL_assertion(new_he->twin()->get_aux_is_set(0) && new_he->twin()->get_aux_is_set(1));

    new_he->set_decision(res);
    new_he->twin()->set_decision(res);
    return res;
  }


  // check if the point is on the curve
  // left and right should be the left and right endpoints of the curve cv
  bool is_point_on_curve(const Point_2& p, const X_monotone_curve_2& cv,
                         const Point_2& left, const Point_2& right)
  {
    CGAL_precondition(traits->equal_2_object()
                        (left, traits->construct_min_vertex_2_object()(cv)));

    CGAL_precondition(traits->equal_2_object()
                        (right, traits->construct_max_vertex_2_object()(cv)));
                        
    // we should check if the point is in the x range of the curve,
    // and if so, if it lies on it
    if ((traits->compare_x_2_object()(p, left) != SMALLER) &&

        (traits->compare_x_2_object()(p, right) != LARGER) &&
        (traits->compare_y_at_x_2_object()(p, cv) == EQUAL))
      return true;
    return false;
  }
  
  // this observer is used in the process of resolving a face
  // this observer should copy the faces' indication when a face is split
  // so we can later identify all the faces that form the original given face
  // it also should remember the edges of the face, that are also projected intersections
  class Copied_face_observer : public Md_observer

  {
  public:
    typedef typename Minimization_diagram_2::Face_handle         Face_handle;

    typedef typename Minimization_diagram_2::Halfedge_handle     Halfedge_handle;

    typedef typename Minimization_diagram_2::X_monotone_curve_2  X_monotone_curve_2;

    Copied_face_observer(Halfedges_map  &map_h)
      : map_halfedges(map_h)
    {
    }
    virtual ~Copied_face_observer() {}

    void set_elements_collections(Halfedges_hash& boundary,
                                  Halfedges_hash& specialh,
                                  Halfedges_hash_w_type& newh,
                                  Faces_hash& parts,
                                  Vertices_hash& boundaryv,
                                  Vertices_hash& specialv,
								  Vertices_to_edges_map& v_to_h)
    {
      boundary_halfedges = &boundary;
      special_edges = &specialh;
      new_edges = &newh;
      face_parts = &parts;
      boundary_vertices = &boundaryv;
      special_vertices = &specialv;
	  vertices_to_halfedges = &v_to_h;
    }
  
    virtual void after_split_face(Face_handle org_f,
                                  Face_handle new_f, bool)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in Copied_face_observer after_split_face" << std::endl;
      #endif
      // keep track of the face parts
      if (face_parts->is_defined(org_f))
        (*face_parts)[new_f] = face_parts->default_value();
     }

    virtual void after_split_edge(Halfedge_handle org_he,
                                  Halfedge_handle new_he)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in Copied_face_observer after_split_edge" << std::endl;
      #endif

      // take care of special edges that were split
      if (special_edges->is_defined(org_he))

      {
        // if original edge was in the set, then now both split parts should
        // be in the set
        (*special_edges)[new_he] = special_edges->default_value();
        (*special_edges)[new_he->twin()] = special_edges->default_value();
      }

      // take care of new edges that were split
      if (new_edges->is_defined(org_he))
      {
        (*new_edges)[new_he] = (*new_edges)[org_he];
        (*new_edges)[new_he->twin()] = (*new_edges)[org_he];
      }

      // take care for boundary edges
      if (boundary_halfedges->is_defined(org_he))
      {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "original boundary edge was split" << std::endl;
        #endif

        (*boundary_halfedges)[new_he] = boundary_halfedges->default_value();
        (*boundary_halfedges)[new_he->twin()] =
                                        boundary_halfedges->default_value();

        // the new created vertex is a special vertex since it lies on the boundary
        // of the face, and it is of a projected intersection
        // we are interested in it only if the split halfedge is "data from face"
        CGAL_assertion(map_halfedges.is_defined(org_he));
        CGAL_assertion(org_he->target() == new_he->source());

        if ((map_halfedges[org_he])->get_has_equal_aux_data_in_face(0) &&
            (map_halfedges[org_he])->get_has_equal_aux_data_in_face(1))
          (*special_vertices)[org_he->target()] =
                              special_vertices->default_value();


        // update the boundary vertices collection
        (*boundary_vertices)[org_he->target()] =
                             boundary_vertices->default_value();


    		// update the vertices to halfedges collection
    		Halfedge_handle correct_side_he;
    		if (face_parts->is_defined(org_he->face()))
          correct_side_he = org_he;
    		else
    		{
          CGAL_assertion(face_parts->is_defined(new_he->twin()->face()));
    		  // new_he->twin() is directed as org_he, so on the boundary pointing
    		  // inside the face, and has the new vertex as target
    		  CGAL_assertion(org_he->target() == new_he->twin()->target());
          correct_side_he = new_he->twin();
     		}
		    // set the new vertex
        (*vertices_to_halfedges)[org_he->target()] = correct_side_he;

		    // update the old vertices (only one needs update, unless it is antenna)
		    CGAL_assertion(vertices_to_halfedges->is_defined(correct_side_he->source()) &&
			                 vertices_to_halfedges->is_defined(correct_side_he->next()->target()));
    		(*vertices_to_halfedges)[correct_side_he->next()->target()] = correct_side_he->next();

    		if (correct_side_he == org_he && face_parts->is_defined(org_he->twin()->face()))
    		  (*vertices_to_halfedges)[org_he->source()] = org_he->twin();

      }
    }

  protected:
    Halfedges_hash *boundary_halfedges;
    Halfedges_hash *special_edges;
    Halfedges_hash_w_type *new_edges;
    Faces_hash     *face_parts;
    Vertices_hash  *boundary_vertices;
    Vertices_hash  *special_vertices;
	Vertices_to_edges_map *vertices_to_halfedges;

    Halfedges_map  &map_halfedges;
  };


  // this observer is used in the process of resolving a face
  // it listens to what happpens in the copied arrangement, and copies back
  // the actions to result arrangements very efficiently
  class Copy_observer : public Md_observer
  {
  public:
    typedef typename Minimization_diagram_2::Face_handle         Face_handle;
    typedef typename Minimization_diagram_2::Halfedge_handle     Halfedge_handle;
    typedef typename Minimization_diagram_2::Vertex_handle       Vertex_handle;
    typedef typename Minimization_diagram_2::Point_2             Point_2;
    typedef typename Minimization_diagram_2::X_monotone_curve_2  X_monotone_curve_2;
    typedef typename Minimization_diagram_2::Ccb_halfedge_circulator
                                                        Ccb_halfedge_circulator;

    Copy_observer(Minimization_diagram_2& small,
                  Minimization_diagram_2& big,
                  Halfedges_map& map_h,
                  Vertices_map&  map_v,
                  Faces_map&     map_f)
      : small_arr(small), big_arr(big),
        big_arr_accessor(big_arr),
        map_halfedges(map_h),
        map_vertices(map_v),
        map_faces(map_f)

    {
    }
    
    virtual ~Copy_observer() {}

    virtual void before_create_vertex (const Point_2& /* p */)
    {}
    virtual void after_create_vertex (Vertex_handle v)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3

        std::cout << "in Copy_observer after_create_vertex" << std::endl;
      #endif
      
      // should create a new vertex with v->point() inside
      Vertex_handle new_v = big_arr_accessor.create_vertex(v->point());
      // save a mapping between the 2 vertices
      map_vertices[v] = new_v;
      // add indication of a new vertex (that is not connected to anything,
      // and is also no isolated)
      new_vertices.push_back(v);
    }

    virtual void before_create_edge (const X_monotone_curve_2& /* c */,
  				                           Vertex_handle v1,
  				                           Vertex_handle v2)
    {
      // save state for after_create_edge event
      create_edge_v1 = v1;
      create_edge_v2 = v2;

      is_in_relocate = false;
    }

    virtual void after_create_edge (Halfedge_handle e)

    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in Copy_observer after_create_edge" << std::endl;
      #endif
      // a new edge e was created in small_arr, we should create a corresponing
      // edge in big_arr
      CGAL_assertion(map_vertices.is_defined(create_edge_v1));
      CGAL_assertion(map_vertices.is_defined(create_edge_v2));
      CGAL_assertion(new_vertices.size() <= 2);

      Vertex_handle big_v1 = map_vertices[create_edge_v1];
      Vertex_handle big_v2 = map_vertices[create_edge_v2];
      
      // should check if v1 and v2 are new or old
      // if we have 2 new vertices, they must be new.
      // if we have only one, we should check which is new
      bool v1_is_new = false, v2_is_new = false;
      if (new_vertices.size() == 1)
      {
        if (new_vertices.back() == create_edge_v1)
          v1_is_new = true;
        else
        {
          CGAL_assertion(new_vertices.back() == create_edge_v2);
          v2_is_new = true;
        }          
      }
      if (new_vertices.size() == 2)
      {
        v1_is_new = true;
        v2_is_new = true;
      }
      new_vertices.clear();

      // just to make sure we have the halfedge in the same direction as
      // we got in the before event
      CGAL_assertion(e->source() == create_edge_v1);
      Halfedge_handle he = ((e->source() == create_edge_v1) ? e : e->twin());

      // if an endpoint is not new, but is isolated, we should remove it from
      // its face's isolated vertices list, and treat it as new
      if (!v1_is_new && big_v1->is_isolated())
      {
        //Face_handle f = big_v1->face(); //big_arr.incident_face(big_v1);
        //big_arr_accessor.find_and_erase_isolated_vertex(f, big_v1);
        big_arr_accessor.remove_isolated_vertex_ex(big_v1);
        v1_is_new = true;
      }
      if (!v2_is_new && big_v2->is_isolated())
      {
        //Face_handle f = big_v2->face(); //big_arr.incident_face(big_v2);
        //big_arr_accessor.find_and_erase_isolated_vertex(f, big_v2);
        big_arr_accessor.remove_isolated_vertex_ex(big_v2);
        v2_is_new = true;
      }

      // now use the approppriate method to insert the new edge
      if (v1_is_new && v2_is_new)
      {
        // if both vertices are new - use the O(1) operation
        // _insert_in_face_interior (in the face mapped to by he->face())
        CGAL_assertion(map_faces.is_defined(he->face()));

        Face_handle big_face = map_faces[he->face()];
        Halfedge_handle new_he =
                 big_arr_accessor.insert_in_face_interior_ex(he->curve(),
                                                             big_face,
                                                             big_v1, big_v2,
                                                             he->direction());

        // update mapping of new edge
        // new_he is directed from big_v1 to big_v2, and he is directed from
        // create_edge_v1 to create_edge_v2, so he is mapped to new_he
        map_halfedges[he] = new_he;
        map_halfedges[he->twin()] = new_he->twin();
      }
      else if (!v1_is_new && !v2_is_new)
      {
        // if both vertices are old - use _insert_at_vertices
        // this is a linear action by the size of the faces involved
        // we can get relevant prev halfedges from he
        Halfedge_handle prev1 = he->prev();
        Halfedge_handle prev2 = he->twin()->prev();

        CGAL_assertion(map_halfedges.is_defined(prev1));
        CGAL_assertion(map_halfedges.is_defined(prev2));

        Halfedge_handle big_prev1 = map_halfedges[prev1];
        Halfedge_handle big_prev2 = map_halfedges[prev2];

        bool new_face;
        Halfedge_handle new_he =
                 big_arr_accessor.insert_at_vertices_ex(he->curve(),

                                                        big_prev1, big_prev2,
                                                        he->direction(),
                                                        new_face);
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "in copy_observer, use insert at vertices" << std::endl;
          std::cout << (!new_face ? "no " : "") <<  "new face created" << std::endl;
        #endif
        // new_he should be directed as he

        CGAL_assertion(map_vertices.is_defined(he->source()) &&
                       map_vertices[he->source()] == new_he->source() &&
                       map_vertices.is_defined(he->target()) &&
                       map_vertices[he->target()] == new_he->target());

        // update mapping of new edge
        map_halfedges[he] = new_he;
        map_halfedges[he->twin()] = new_he->twin();

        // make sure that the old face is mapped already
        CGAL_assertion(map_faces.is_defined(he->twin()->face()) &&
                       map_faces[he->twin()->face()] == new_he->twin()->face());

        // if a new face was created update its mapping too
        // the new face is the incident face of he
        if (new_face)
        {
          map_faces[he->face()] = new_he->face();
          // save state for move_hole/move_isolated_vertex events
          is_in_relocate = true;
        }


        // make sure the face is correctly mapped
        CGAL_assertion(map_faces.is_defined(he->face()) &&
                       map_faces[he->face()] == new_he->face());
      }
      else
      {
        // only one vertex is new - use the O(1) operation _insert_from_vertex
        // we can get the relevant prev halfedge from e
        Halfedge_handle prev = he->prev();
        CGAL_assertion(map_halfedges.is_defined(prev));
        Halfedge_handle big_prev = map_halfedges[prev];
        Halfedge_handle new_he;
        if (!v1_is_new)
        {
          new_he = big_arr_accessor.insert_from_vertex_ex(he->curve(),
                                                          big_prev, big_v2,
                                                          he->direction());
          // update mapping of new edge
          // new_he is directed from big_v1 to big_v2 as he
          map_halfedges[he] = new_he;
          map_halfedges[he->twin()] = new_he->twin();          
        }
        else
        {
          new_he = big_arr_accessor.insert_from_vertex_ex(he->curve(),
                                                          big_prev, big_v1,
                                                          he->direction());
          // update mapping of new edge
          // new_he is directed from big_v2 to big_v1 opposite of he
          map_halfedges[he] = new_he->twin();
          map_halfedges[he->twin()] = new_he;

        }
      }
    }

    virtual void before_split_edge (Halfedge_handle e,
                                    Vertex_handle v,
                                    const X_monotone_curve_2& /* c1 */,
                                    const X_monotone_curve_2& /* c2 */)
    {
      // save state info for using _split_edge in after event
      split_v = v;
      split_e = e;
    }
    virtual void after_split_edge (Halfedge_handle e1,
                                   Halfedge_handle e2)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in Copy_observer after_split_edge" << std::endl;
      #endif
      // find the corresponding split vertex in big_arr
      CGAL_assertion(map_vertices.is_defined(split_v));
      Vertex_handle big_v = map_vertices[split_v];

      // make sure it is the only new vertex right now
      CGAL_assertion(new_vertices.size() == 1 &&
                     new_vertices.back() == split_v);
      new_vertices.pop_back();

      // find the edge to split in big_arr
      CGAL_assertion(map_halfedges.is_defined(split_e));
      Halfedge_handle big_e = map_halfedges[split_e];

      // use the O(1) operation _split_edge      
      Halfedge_handle big_e1 =
               big_arr_accessor.split_edge_ex(big_e, big_v,
                                              e1->curve(), e2->curve());

      Halfedge_handle big_e2 = big_e1->next();
      
      // update mapping of new halfedges
      // big_e1 is directed at big_v, as e1 is directed at split_v -
      // these are supposed to be mapped
      CGAL_assertion(map_halfedges.is_defined(e1) &&
                     map_halfedges[e1] == big_e1);
      // should update the mapping of the second halfedge     
      map_halfedges[e2] = big_e2;
      map_halfedges[e2->twin()] = big_e2->twin();
    }

    virtual void before_add_isolated_vertex (Face_handle f,
  					   Vertex_handle /* v */)
    {
      saved_face = f;
    }
    virtual void after_add_isolated_vertex (Vertex_handle v)
    {
      // make sure it is the only new vertex right now
      CGAL_assertion(new_vertices.size() == 1 &&
                     new_vertices.back() == v);
      new_vertices.pop_back();


      CGAL_assertion(map_vertices.is_defined(v));
      CGAL_assertion(map_faces.is_defined(saved_face));
      
      // find features in big_arr
      Vertex_handle big_v = map_vertices[v];

      Face_handle   big_face = map_faces[saved_face];
      

      // can use O(1) operation _insert_isolated_vertex
      big_arr_accessor.insert_isolated_vertex(big_face, big_v);
    }

    virtual void before_move_hole (Face_handle from_f,
                                   Face_handle to_f,

                                   Ccb_halfedge_circulator h)
    {
      // should be used after insert_at_vertices which creates a new face
      CGAL_assertion(is_in_relocate);
      move_from = from_f;
      move_to = to_f;
    }
    virtual void after_move_hole (Ccb_halfedge_circulator h)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in Copy_observer after_move_hole" << std::endl;
      #endif
      CGAL_assertion(map_faces.is_defined(move_from));
      CGAL_assertion(map_faces.is_defined(move_to));
      CGAL_assertion(map_halfedges.is_defined(h));

      Face_handle big_from_f  = map_faces[move_from];
      Face_handle big_to_f = map_faces[move_to];
      Ccb_halfedge_circulator big_h = (map_halfedges[h])->ccb();

      Ccb_halfedge_circulator big_ccb = big_h;
      
      big_arr_accessor.move_hole(big_from_f, big_to_f, big_ccb);

    }

    virtual void before_move_isolated_vertex (Face_handle from_f,
  					      Face_handle to_f,
  					      Vertex_handle v)
    {
      // should be used after insert_at_vertices which creates a new face

      CGAL_assertion(is_in_relocate);
      move_from = from_f;
      move_to = to_f;
    }
    virtual void after_move_isolated_vertex (Vertex_handle v)
    {
      CGAL_assertion(map_faces.is_defined(move_from));
      CGAL_assertion(map_faces.is_defined(move_to));
      CGAL_assertion(map_vertices.is_defined(v));

      Face_handle big_from_f  = map_faces[move_from];
      Face_handle big_to_f = map_faces[move_to];
      Vertex_handle big_v = map_vertices[v];

      big_arr_accessor.move_isolated_vertex(big_from_f, big_to_f, big_v);
    }

  protected:
    Minimization_diagram_2& small_arr;
    Minimization_diagram_2& big_arr;
    Md_accessor             big_arr_accessor;

    // mappings between small_arr features to big_arr features
    Halfedges_map& map_halfedges;
    Vertices_map&  map_vertices;
    Faces_map&     map_faces;
    std::deque<Vertex_handle> new_vertices;
    
    // state for actions
    Vertex_handle create_edge_v1;
    Vertex_handle create_edge_v2;
    Vertex_handle split_v;
    Halfedge_handle split_e;
    Face_handle   saved_face;
    Face_handle   move_from;
    Face_handle   move_to;    
    bool is_in_relocate;
    
  };

  // A zone visitor for the Minimization Diagram which only inserts
  // parts of the curve which are inside a given face
  // it also remembers those parts which overlap the boundary of the original face
  class Copied_face_zone_visitor
  {
  public:
    typedef typename Minimization_diagram_2::Vertex_handle       Vertex_handle;
    typedef typename Minimization_diagram_2::Halfedge_handle     Halfedge_handle;
    typedef typename Minimization_diagram_2::Face_handle         Face_handle;


    typedef typename Minimization_diagram_2::Point_2             Point_2;
    typedef typename Minimization_diagram_2::X_monotone_curve_2  X_monotone_curve_2;

    typedef std::pair<Halfedge_handle, bool>                     Result;

    Copied_face_zone_visitor(Minimization_diagram_2& result,
                             Minimization_diagram_2& copied,
                             Face_handle orig_face,
                             Face_handle copied_face,
                             Halfedges_map  &map_h,

                             Vertices_map   &map_v,
                             Faces_map      &map_f,
                             Halfedges_list &se, // special edges
                             Halfedges_w_type_list &new_edges,
                             Faces_list     &face_parts,
                             Vertices_list  &sv, // special vertices
                             Self* p)
          : copied_arr(copied),
            result_arr(result),
            result_original_face(orig_face),
            map_halfedges(map_h),
            map_vertices(map_v),
            map_faces(map_f),
            result_special_edges(se),
            result_new_edges(new_edges),
            result_face_parts(face_parts),
            result_special_vertices(sv),
            md_copy_observer(copied, result, map_h, map_v, map_f),
            md_observer(map_h),
            parent(p)
    {
      // init maps
      copied_face_parts[copied_face] = copied_face_parts.default_value();

      Halfedge_iterator hi = copied_arr.halfedges_begin();
      for(; hi != copied_arr.halfedges_end(); ++hi)
      {
        copied_arr_boundary_halfedges[hi] =
                   copied_arr_boundary_halfedges.default_value();
        if (hi->face() == copied_face)
		  copied_vertices_to_halfedges[hi->target()] = hi;
	  }
                   
      Vertex_iterator vi = copied_arr.vertices_begin();
      for(; vi != copied_arr.vertices_end(); ++vi)
      {
        copied_arr_orig_vertices[vi] =
                   copied_arr_orig_vertices.default_value();

		if (vi->is_isolated())
		{
          CGAL_assertion(vi->face() == copied_face);
          copied_vertices_to_halfedges[vi] = Halfedge_handle(NULL);
		}
		else
		  CGAL_assertion(copied_vertices_to_halfedges.is_defined(vi));
      }

      // init observers
      md_copy_observer.attach(copied_arr);
      
      md_observer.set_elements_collections(copied_arr_boundary_halfedges,
                                           copied_arr_special_edges,
                                           copied_arr_new_edges,
                                           copied_face_parts,
                                           copied_arr_new_boundary_vertices,
                                           copied_arr_special_vertices,
										   copied_vertices_to_halfedges);
      md_observer.attach(copied_arr);
    }

    virtual ~Copied_face_zone_visitor() { }

    // the zone visitor functions

    /*! Initialize the visitor with an arrangement object. */
    void init (Minimization_diagram_2 *arr)
    {
      CGAL_assertion(&copied_arr == arr);
      insert_visitor.init(arr);
    }

    /*!
     * Handle the a subcurve located in the interior of a given face.
     * \param cv The subcurve.
     * \param face The face containing cv's interior.
     * \param left_v The vertex that corresponds to the left endpoint of cv
     *               (or an invalid handle if no such arrangement vertex exists).
     * \param left_he The halfedge that contains the left endpoint of cv
     *               (or an invalid handle if no such halfedge exists).
     * \param right_v The vertex that corresponds to the right endpoint of cv
     *               (or an invalid handle if no such arrangement vertex exists).
     * \param right_he The halfedge that contains the right endpoint of cv
     *                 (or an invalid handle if no such halfedge exists).
     * \return A handle to the halfedge obtained from the insertion of the
     *         subcurve into the arrangement.
     */
    Result found_subcurve (const X_monotone_curve_2& cv,
                           Face_handle face,
                           Vertex_handle left_v, Halfedge_handle left_he,
                           Vertex_handle right_v, Halfedge_handle right_he)
    {
      // insert the curve only if the face is ok
      if (is_face_ok(face))
      {
        Result base_result = insert_visitor.found_subcurve(cv, face,
                                                           left_v, left_he,
                                                           right_v, right_he);
        // update the collection of newly added edges
        Halfedge_handle new_he = base_result.first;
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "found a new edge " << new_he->curve() << std::endl;
          std::cout << "from " << new_he->source()->point() << std::endl;
        #endif
        copied_arr_new_edges[new_he] = itype;
        copied_arr_new_edges[new_he->twin()] = itype;
        
        // take care for special vertices. the split vertices are always special,
        // and this is taken care of in the after_split event.

        // here we should update the original vertices that consolidate with the
        // new subcurve
        if (copied_arr_orig_vertices.is_defined(new_he->source()))
          copied_arr_special_vertices[new_he->source()] =
              copied_arr_special_vertices.default_value();

        if (copied_arr_orig_vertices.is_defined(new_he->target()))
          copied_arr_special_vertices[new_he->target()] =
              copied_arr_special_vertices.default_value();

    		// we should set the halfedge-face, halfedge-target 
		    // and target-face aux flags on the new edge (of result)
    		Halfedge_handle result_new_he = map_halfedges[new_he];
    		// it is clear that the halfedge-face are all true
        result_new_he->set_is_equal_aux_data_in_face(0, true);
        result_new_he->set_is_equal_aux_data_in_face(1, true);

        result_new_he->twin()->set_is_equal_aux_data_in_face(0, true);
        result_new_he->twin()->set_is_equal_aux_data_in_face(1, true);
        result_new_he->set_has_equal_aux_data_in_face(0, true);
        result_new_he->set_has_equal_aux_data_in_face(1, true);
        result_new_he->twin()->set_has_equal_aux_data_in_face(0, true);
        result_new_he->twin()->set_has_equal_aux_data_in_face(1, true);
        // for the halfedge-target flags, if the vertex is a boundary vertex
    		// we should use its boundary halfedge as intermediary between the face
    		// and the vertex (or the vertex info if it was isolated)
    		// otherwise, we set flags to true since it is a new vertex inside the
    		// original face, and have same aux data as all face parts
    		if (is_boundary_vertex(new_he->target()))
    		{
          Vertex_handle cur_t = new_he->target();
          CGAL_assertion(copied_vertices_to_halfedges.is_defined(cur_t));
          Halfedge_handle copied_b_he = copied_vertices_to_halfedges[cur_t];
    		  if (copied_b_he == Halfedge_handle(NULL))
          {
            // this was an isolated vertex, which we touch
            // since we have in the new edge aux sources as in the face,
            // we can copy the vertex-face flags from the vertex
            result_new_he->set_is_equal_aux_data_in_target
    		           (0, cur_t->get_is_equal_aux_data_in_face(0));
            result_new_he->set_is_equal_aux_data_in_target
    		           (1, cur_t->get_is_equal_aux_data_in_face(1));
            result_new_he->set_has_equal_aux_data_in_target
    		           (0, cur_t->get_has_equal_aux_data_in_face(0));
            result_new_he->set_has_equal_aux_data_in_target
    		           (1, cur_t->get_has_equal_aux_data_in_face(1));
            result_new_he->set_has_equal_aux_data_in_target_and_face
    		           (0, cur_t->get_has_equal_aux_data_in_face(0));
            result_new_he->set_has_equal_aux_data_in_target_and_face
    		           (1, cur_t->get_has_equal_aux_data_in_face(1));
    		  }
    		  else
    		  {
            CGAL_assertion(copied_b_he->target() == cur_t);
     	      CGAL_assertion(is_boundary_edge(copied_b_he));
            Halfedge_handle b_he = map_halfedges[copied_b_he];

            #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
     	        std::cout << "b_he=" << b_he->source()->point() << " --> " << b_he->target()->point();
            #endif

            bool flag;
       			flag = (b_he->get_is_equal_aux_data_in_face(0) &&
       				      b_he->get_is_equal_aux_data_in_target(0));
       			result_new_he->set_is_equal_aux_data_in_target(0, flag);

       			flag = (b_he->get_is_equal_aux_data_in_face(1) &&
       				      b_he->get_is_equal_aux_data_in_target(1));
       			result_new_he->set_is_equal_aux_data_in_target(1, flag);

            flag = b_he->get_has_equal_aux_data_in_target_and_face(0);
      			CGAL_assertion(flag == parent->has_equal_aux_data(0, b_he->face(), b_he->target()));
      			result_new_he->set_has_equal_aux_data_in_target(0, flag);
            result_new_he->set_has_equal_aux_data_in_target_and_face(0, flag);

            flag = b_he->get_has_equal_aux_data_in_target_and_face(1);
      			CGAL_assertion(flag == parent->has_equal_aux_data(1, b_he->face(), b_he->target()));
       			result_new_he->set_has_equal_aux_data_in_target(1, flag);
            result_new_he->set_has_equal_aux_data_in_target_and_face(1, flag);

    		  }
    		}
    		else // not a boundary vertex
        {
          result_new_he->set_is_equal_aux_data_in_target(0, true);
          result_new_he->set_is_equal_aux_data_in_target(1, true);
		      // the face's data is not empty - so it is ok to set "true" here
          result_new_he->set_has_equal_aux_data_in_target(0, true);
          result_new_he->set_has_equal_aux_data_in_target(1, true);
          result_new_he->set_has_equal_aux_data_in_target_and_face(0, true);
          result_new_he->set_has_equal_aux_data_in_target_and_face(1, true);
    		}

    		if (is_boundary_vertex(new_he->source()))
    		{
          Vertex_handle cur_t = new_he->source();
          CGAL_assertion(copied_vertices_to_halfedges.is_defined(cur_t));
          Halfedge_handle copied_b_he = copied_vertices_to_halfedges[cur_t];
    		  if (copied_b_he == Halfedge_handle(NULL))
          {
            // this was an isolated vertex, which we touch
            // since we have in the new edge aux sources as in the face,
            // we can copy the vertex-face flags from the vertex
            result_new_he->twin()->set_is_equal_aux_data_in_target
    		           (0, cur_t->get_is_equal_aux_data_in_face(0));
            result_new_he->twin()->set_is_equal_aux_data_in_target
    		           (1, cur_t->get_is_equal_aux_data_in_face(1));
            result_new_he->twin()->set_has_equal_aux_data_in_target
    		           (0, cur_t->get_has_equal_aux_data_in_face(0));
            result_new_he->twin()->set_has_equal_aux_data_in_target
    		           (1, cur_t->get_has_equal_aux_data_in_face(1));
            result_new_he->twin()->set_has_equal_aux_data_in_target_and_face
    		           (0, cur_t->get_has_equal_aux_data_in_face(0));
            result_new_he->twin()->set_has_equal_aux_data_in_target_and_face
    		           (1, cur_t->get_has_equal_aux_data_in_face(1));
    		  }
    		  else
    		  {
            CGAL_assertion(copied_b_he->target() == cur_t);
    	      CGAL_assertion(is_boundary_edge(copied_b_he));
            Halfedge_handle b_he = map_halfedges[copied_b_he];

            #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
              std::cout << "b_he=" << b_he->source()->point() << " --> " << b_he->target()->point();
            #endif
            bool flag;
      			flag = (b_he->get_is_equal_aux_data_in_face(0) &&
      				      b_he->get_is_equal_aux_data_in_target(0));
      			result_new_he->twin()->set_is_equal_aux_data_in_target(0, flag);

      			flag = (b_he->get_is_equal_aux_data_in_face(1) &&
      				      b_he->get_is_equal_aux_data_in_target(1));
      			result_new_he->twin()->set_is_equal_aux_data_in_target(1, flag);

            flag = b_he->get_has_equal_aux_data_in_target_and_face(0);
      			CGAL_assertion(flag == parent->has_equal_aux_data(0, b_he->face(), b_he->target()));
      			result_new_he->twin()->set_has_equal_aux_data_in_target(0, flag);
            result_new_he->twin()->set_has_equal_aux_data_in_target_and_face(0, flag);

            flag = b_he->get_has_equal_aux_data_in_target_and_face(1);
      			CGAL_assertion(flag == parent->has_equal_aux_data(1, b_he->face(), b_he->target()));
       			result_new_he->twin()->set_has_equal_aux_data_in_target(1, flag);
            result_new_he->twin()->set_has_equal_aux_data_in_target_and_face(1, flag);
    		  }
    		}
    		else
        {
          result_new_he->twin()->set_is_equal_aux_data_in_target(0, true);
          result_new_he->twin()->set_is_equal_aux_data_in_target(1, true);
          result_new_he->twin()->set_has_equal_aux_data_in_target(0, true);
          result_new_he->twin()->set_has_equal_aux_data_in_target(1, true);
          result_new_he->twin()->set_has_equal_aux_data_in_target_and_face(0, true);
          result_new_he->twin()->set_has_equal_aux_data_in_target_and_face(1, true);
    		}

        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
    		  std::cout << "set halfedge-vertex flags:"
    			          << std::endl;

    		  std::cout << "is equal(0) = " << result_new_he->get_is_equal_aux_data_in_target(0)
    			          << " is_equal(1) = " << result_new_he->get_is_equal_aux_data_in_target(1)
    			          << " has_equal(0) = " << result_new_he->get_has_equal_aux_data_in_target(0)
    			          << " has_equal(1) = " << result_new_he->get_has_equal_aux_data_in_target(1)
    			          << std::endl;
    		  std::cout << "for twin: " << std::endl;
    		  std::cout << "is equal(0) = " << result_new_he->twin()->get_is_equal_aux_data_in_target(0)
    			          << " is_equal(1) = " << result_new_he->twin()->get_is_equal_aux_data_in_target(1)
    			          << " has_equal(0) = " << result_new_he->twin()->get_has_equal_aux_data_in_target(0)
    			          << " has_equal(1) = " << result_new_he->twin()->get_has_equal_aux_data_in_target(1)
    			          << std::endl;

        #endif
        return base_result;
      }
      else
      {
        // we don't insert the subcurve, but it might touch a vertex of the
        // face's boundary - we need to check it and identify special vertices
        if (left_v != Vertex_handle(NULL) &&
            copied_arr_orig_vertices.is_defined(left_v))
          copied_arr_special_vertices[left_v] =
              copied_arr_special_vertices.default_value();

        if (right_v != Vertex_handle(NULL) &&
            copied_arr_orig_vertices.is_defined(right_v))
          copied_arr_special_vertices[right_v] =
              copied_arr_special_vertices.default_value();
        
        Halfedge_handle invalid_hh;
        return Result (invalid_hh, false);
      }
    }

    /*!
     * Handle the a subcurve that overlaps a given edge.
     * \param cv The overlapping subcurve.
     * \param he The overlapped halfedge (directed from left to right).
     * \param left_v The vertex that corresponds to the left endpoint of cv
     *               (or an invalid handle if no such arrangement vertex exists).
     * \param right_v The vertex that corresponds to the right endpoint of cv
     *               (or an invalid handle if no such arrangement vertex exists).
     * \return A handle to the halfedge obtained from the insertion of the
     *         overlapping subcurve into the arrangement.
     */
    Result found_overlap (const X_monotone_curve_2& cv,
                          Halfedge_handle he,
                          Vertex_handle left_v, Vertex_handle right_v)
    {
      // check if the halfedge is the boundary of the original face
      // (here we assume that this indication is dealt with in an observer
      //  attached to the md, and implements split_edge)
      bool is_boundary = is_boundary_edge(he);

      // use insert_visitor to get the halfedge with the overlap
      Result base_res = insert_visitor.found_overlap(cv, he, left_v, right_v);

      Halfedge_handle overlap_he = base_res.first;
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "In copied face zone visitor: found an overlap"
                  << overlap_he->curve() << std::endl;
      #endif

      // take care for special vertices. the split vertices are always special,
      // and this is taken care of in the after_split event.
      // here we should update the original vertices that consolidate with the
      // new subcurve
        if (copied_arr_orig_vertices.is_defined(overlap_he->source()))
          copied_arr_special_vertices[overlap_he->source()] =
              copied_arr_special_vertices.default_value();

        if (copied_arr_orig_vertices.is_defined(overlap_he->target()))
          copied_arr_special_vertices[overlap_he->target()] =
              copied_arr_special_vertices.default_value();

      if (!is_boundary)
        return base_res;

      // if he is a boundary edge, it is a special edge
      if (is_boundary)
      {
        #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
          std::cout << "In copied face zone visitor: found a special edge" << std::endl;
        #endif
        copied_arr_special_edges[overlap_he] =
                                 copied_arr_special_edges.default_value();
        copied_arr_special_edges[overlap_he->twin()] =
                                 copied_arr_special_edges.default_value();
       }
      return base_res;
    }

    /*!                                                   

     * Handle point that lies inside a given face.
     * \param p The point.
     * \param face The face inside which the point lies.
     * \return A handle to the new vertex obtained from the insertion of the
     *         point into the face, or invalid handle if the point wasn't
     *         inserted to the arrangement.
     */
    Vertex_handle found_point_in_face(const Point_2& p, Face_handle face)
    {
      // p lies inside a face: Insert it as an isolated vertex it the interior of
      // this face.
      Vertex_handle vh_for_p;
      if (is_face_ok(face))
      {
        vh_for_p = copied_arr.insert_in_face_interior(p, face);

      	// now should set the is_equal and has_equal flags
      	CGAL_assertion(map_vertices.is_defined(vh_for_p));
      	Vertex_handle result_new_v = map_vertices[vh_for_p];
      	result_new_v->set_is_equal_aux_data_in_face(0, true);
      	result_new_v->set_is_equal_aux_data_in_face(1, true);
        result_new_v->set_has_equal_aux_data_in_face(0, true);
      	result_new_v->set_has_equal_aux_data_in_face(1, true);
      }
      return vh_for_p;
    }

    /*!

     * Handle point that lies on a given edge.
     * \param p The point.
     * \param he The edge on which the point lies.
     * \return A handle to the new vertex obtained from the insertion of the
     *         point into the edge, or invalid handle if the point wasn't
     *         inserted to the arrangement.
     */
    Vertex_handle found_point_on_edge(const Point_2& p, Halfedge_handle he)
    {
      // p lies in the interior of an edge: Split this edge to create a new
      // vertex associated with p.
      X_monotone_curve_2  sub_cv1, sub_cv2;
      Halfedge_handle     split_he;
      copied_arr.get_traits()->split_2_object() (he->curve(), p, sub_cv1, sub_cv2);
      split_he = copied_arr.split_edge (he, sub_cv1, sub_cv2);

      // if the edge is a boundary edge, then the new vertex is a special vertex
      // and this is taken care of in the after_split event

      // TODO: should we update some is_equal / has_equal flags? 
      // I think that no, because it is handled in the after_split event

      // The new vertex is the target of the returned halfedge.

      return split_he->target();
    }

    /*!
     * Handle point that lies on a given vertex.
     * \param p The point.
     * \param v The vertex on which the point lies.
     * \return A handle to the new vertex obtained from the modifying
     *         the existing vertex.
     */
    Vertex_handle found_point_on_vertex(const Point_2& p, Vertex_handle v)
    {
      // if the vertex is a boundary vertex, then it is a special vertex
      // if it was created by split of a boundary edge, then it is already
      // marked as special. we need to mark it as special if it is an original
      // vertex
      if (copied_arr_orig_vertices.is_defined(v))
        copied_arr_special_vertices[v] =
                                   copied_arr_special_vertices.default_value();

      return copied_arr.modify_vertex (v, p);
    }

    /*!
     * Update all the output collections using the internal data saved during
     * the previous inserts.
     * Should be called after all inserts have finished.
     */
    void finish()
    {
      // result_special_edges
      // result_new_edges
      Halfedge_iterator hi = copied_arr.halfedges_begin();
      for(; hi != copied_arr.halfedges_end(); ++hi, ++hi)
      {
        Halfedge_handle h = hi;
        CGAL_assertion(map_halfedges.is_defined(h) &&
                       map_halfedges.is_defined(h->twin()));


        // we need only one of the twin halfedges to represent the new edge
        if (copied_arr_new_edges.is_defined(h))
          result_new_edges.push_back(std::make_pair(map_halfedges[h],
                                                    copied_arr_new_edges[h]));

        if (copied_arr_special_edges.is_defined(h))
        {
          // we need the halfedge that its incident face is inside the original
          // face
          Face_handle f1 = h->face(), f2 = h->twin()->face();
          if (copied_face_parts.is_defined(f1))

            result_special_edges.push_back(map_halfedges[h]);
          else
          {
            CGAL_assertion(copied_face_parts.is_defined(f2));
            result_special_edges.push_back(map_halfedges[h->twin()]);
          }
        }

      }

      // result_face_parts
      Face_iterator fi = copied_arr.faces_begin();
      for(; fi != copied_arr.faces_end(); ++fi)
      {
        Face_handle f = fi;
        if (copied_face_parts.is_defined(f))
        {
          CGAL_assertion(map_faces.is_defined(f));
          result_face_parts.push_back(map_faces[f]);
        }
      }
      
      // result_special_vertices
      Vertex_iterator vi = copied_arr.vertices_begin();
      for(; vi != copied_arr.vertices_end(); ++vi)

      {
        Vertex_handle v = vi;
        CGAL_assertion(map_vertices.is_defined(v));
        Vertex_handle result_v = map_vertices[v];
        
        if (copied_arr_orig_vertices.is_defined(v))
        {
          // original vertex should be mapped to a boundary halfedge whose
          // target is the vertex
          CGAL_assertion(copied_vertices_to_halfedges.is_defined(v));
   		  Halfedge_handle inc_he = copied_vertices_to_halfedges[v];
   		  CGAL_assertion(copied_face_parts.is_defined(inc_he->face()));

   		  Halfedge_handle result_inc_he = map_halfedges[inc_he];
          CGAL_assertion(result_inc_he->target() == result_v);
   		  CGAL_assertion(map_faces[inc_he->face()] == result_inc_he->face());

          // original vertex is special if it appears in the special collection
          // and its aux data share equal surfaces with the faces aux data
          if (copied_arr_special_vertices.is_defined(v) &&
              ((result_v->is_isolated() && 
			    parent->has_equal_aux_data_with_face(result_v)) ||
               (!result_v->is_isolated() && 
			    parent->has_equal_aux_data_in_target_and_face(result_inc_he)))) 
          {
            CGAL_assertion(parent->has_equal_aux_data(result_v, result_original_face));
            result_special_vertices.push_back(result_v); 
          }  
        }
        else
        {
          if (!copied_arr_new_boundary_vertices.is_defined(v))
            // new vertex inside the face
            result_special_vertices.push_back(result_v);
          else if (copied_arr_special_vertices.is_defined(v))
            result_special_vertices.push_back(result_v);
        }
      }
    }

    void set_current_intersection_type(Intersection_type t)
    {
      itype = t;
    }
  protected:

    bool is_face_ok(Face_handle face)
    {
      // is this face a part of the original face?
      // check in the copied_face_parts map
      return (copied_face_parts.is_defined(face));
    }

    bool is_boundary_edge(Halfedge_handle he)
    {
      return (copied_arr_boundary_halfedges.is_defined(he));
    }

    bool is_original_boundary_vertex(Vertex_handle v)
    {
      return (copied_arr_orig_vertices.is_defined(v));
    }
    bool is_boundary_vertex(Vertex_handle v)
    {
      return (copied_arr_orig_vertices.is_defined(v) ||
		      copied_arr_new_boundary_vertices.is_defined(v));
    }

  protected:
    // this zone visitor knows how to insert the given subcurves into the
    // minimization diagram. we use it to insert the subcurves we want (which
    // are in the given original face)
    Md_insert_zone_visitor insert_visitor;

    Minimization_diagram_2 &copied_arr;
    Minimization_diagram_2 &result_arr;
    Face_handle            result_original_face;
    
    // mappings between features in the 2 arrangements
    Halfedges_map &map_halfedges;
    Vertices_map  &map_vertices;
    Faces_map     &map_faces;

    // output lists
    Halfedges_list &result_special_edges;
    Halfedges_w_type_list &result_new_edges;
    Faces_list     &result_face_parts;
    Vertices_list  &result_special_vertices;

    // helper collections (for copied_arr features)
    Halfedges_hash        copied_arr_boundary_halfedges;
    Vertices_hash         copied_arr_orig_vertices;
    Vertices_hash         copied_arr_new_boundary_vertices;
    Vertices_to_edges_map copied_vertices_to_halfedges;    

    Halfedges_hash        copied_arr_special_edges;
    Halfedges_hash_w_type copied_arr_new_edges;
    Faces_hash            copied_face_parts;
    Vertices_hash         copied_arr_special_vertices;

    // this observer will take care of the result arrangegment
    Copy_observer        md_copy_observer;

    // this observer will keep all our information in the helper collections
    // during the insert process
    // (the special features info, boundary info, new_edges)
    Copied_face_observer md_observer;
    
    // for using its methods
    Self* parent;

    // current type of interection curve that is inserted
    Intersection_type itype;
  };

  // this minimization diagram observer updates data in new faces created
  class New_faces_observer : public Md_observer

  {
  public:
    typedef typename Minimization_diagram_2::Face_handle Face_handle;

    New_faces_observer(Minimization_diagram_2& arr)
            : Md_observer(arr)
    {}
    virtual ~New_faces_observer() {}

    virtual void after_split_face(Face_handle org_f,
                                  Face_handle new_f,
                                  bool)
    {
      #ifdef CGAL_DEBUG_ENVELOPE_DEQ_3
        std::cout << "in New_faces_observer::after_split_face" << std::endl;
      #endif


      // update the new face's aux_data from original face
      if (org_f->get_aux_is_set(0))
        new_f->set_aux_source(0, org_f->get_aux_source(0));
      if (org_f->get_aux_is_set(1))
        new_f->set_aux_source(1, org_f->get_aux_source(1));
    }

  };

  Traits             *traits;
  bool                own_traits; // Should we eventually free the traits object.

  // measure times for resolve face
  mutable Timer intersection_timer;
  mutable Timer copied_arr_timer;
  mutable Timer zone_timer;
  mutable Timer minimal_face_timer;

  // measure times for resolve edge
  mutable Timer edge_intersection_timer;
  mutable Timer edge_2d_inter_timer;
  mutable Timer edge_split_timer;
};

} //namespace CGAL

#endif
