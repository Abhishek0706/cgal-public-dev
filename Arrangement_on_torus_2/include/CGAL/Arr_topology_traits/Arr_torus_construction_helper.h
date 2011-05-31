// Copyright (c) 2007-2010 Max-Planck-Institute Saarbruecken (Germany), 
// and Tel-Aviv University (Israel).  All rights reserved.
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
// $URL$
// $Id$
// 
//
// Author(s)     : Eric Berberich <eric@mpi-inf.mpg.de>
//                 Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Ron Wein <wein@post.tau.ac.il>
//                 Efi Fogel <efif@post.tau.ac.il>

#ifndef CGAL_ARR_TORUS_CONSTRUCTION_HELPER_H
#define CGAL_ARR_TORUS_CONSTRUCTION_HELPER_H 1 

/*!\file include/CGAL/Arr_topology_traits/Arr_torus_construction_helper.h
 * \brief Definition of the 
 * Arr_torus_construction_helper class-template.
 */

#include <CGAL/config.h>

#include <CGAL/Unique_hash_map.h>

#include <CGAL/Sweep_line_empty_visitor.h>

namespace CGAL {

/*! \class Arr_torus_construction_helper
 * A helper class for the construction sweep-line visitor, suitable
 * for an Arrangement_on_surface_2 instantiated with a topology-traits class
 * for curves on an ellipsoid
 */
template <class Traits_, class Arrangement_, class Event_, class Subcurve_> 
class Arr_torus_construction_helper
{
public:
    typedef Traits_                                         Traits_2;
    typedef Arrangement_                                    Arrangement_2;
    typedef Event_                                          Event;
    typedef Subcurve_                                       Subcurve;
    
    typedef typename Traits_2::X_monotone_curve_2           X_monotone_curve_2;
    typedef typename Traits_2::Point_2                      Point_2;
    
    typedef Sweep_line_empty_visitor<Traits_2, Subcurve, Event>
    Base_visitor;
    
    typedef typename Arrangement_2::Vertex_handle           Vertex_handle;
    typedef typename Arrangement_2::Halfedge_handle         Halfedge_handle;
    typedef typename Arrangement_2::Face_handle             Face_handle;
    
    typedef typename Subcurve::Halfedge_indices_list        Indices_list;
    typedef CGAL::Unique_hash_map<Halfedge_handle, Indices_list> 
                                                          Halfedge_indices_map;
    
protected:
    typedef typename Arrangement_2::Topology_traits         Topology_traits;
    
    typedef typename Topology_traits::Vertex                DVertex;
    typedef typename Topology_traits::Halfedge              DHalfedge;
    
    // Data members:
    
    //! The topology-traits class
    Topology_traits * m_top_traits;
    Arr_accessor<Arrangement_2>  m_arr_access;  // An arrangement accessor.
    
    //! The bounded arrangement face
    Face_handle m_top_face;
    
    //! Indices of the curves that "see" the top face
    Indices_list m_subcurves_at_tf;
    
    //! A pointer to a map of halfedges to indices lists
    // (stored in the visitor class)
    Halfedge_indices_map * m_he_ind_map_p;
    
public:
    /*! Constructor. */
    Arr_torus_construction_helper(Arrangement_2 * arr) :
        m_top_traits(arr->topology_traits()),
        m_arr_access(*arr),
        m_he_ind_map_p(NULL)
    {}
    
    /*! Destructor. */
    virtual ~Arr_torus_construction_helper() {}
    
    /// \name Notification functions.
    //@{
    
    /* A notification issued before the sweep process starts. */
    virtual void before_sweep()
    {
        // Get the bounded face.
        m_top_face = Face_handle(m_top_traits->top_face());
    }
    
    /*! A notification invoked before the sweep-line starts handling the given
     * event.
     */
    virtual void before_handle_event(Event * event)
    {
        Arr_parameter_space ps_x = event->parameter_space_in_x();
        Arr_parameter_space ps_y = event->parameter_space_in_y();
        
        //std::cout << "HELPER: event: " << event->point() << std::endl;
        
        if (ps_x != CGAL::ARR_INTERIOR) {
            //std::cout << "HELPER: before x " << ps_x << std::endl;
            CGAL_assertion(ps_x == CGAL:: ARR_RIGHT_BOUNDARY ||
                           ps_x == CGAL::ARR_LEFT_BOUNDARY);
            CGAL_assertion(ps_y == CGAL::ARR_INTERIOR);
            CGAL_assertion(event->number_of_left_curves() +
                           event->number_of_right_curves() >= 1);
            
            // TODO isolated point

            // added for hole-location
            if (ps_x == CGAL::ARR_LEFT_BOUNDARY && 
                m_top_traits->is_identification_WE_empty()) {
                // The list m_subcurves_at_tf contains all subcurves 
                // whose left endpoint lies between the curve of 
                // identification in WE and the current curve incident 
                // to the WE-identification. In case these subcurves 
                // represent holes, these holes should stay in the 
                // "face to the left" that contains the curve 
                // of identification and we should not keep track of 
                // them in order to later move them to another face.
                //std::cout << "delete sc in tf" << std::endl;
                m_subcurves_at_tf.clear();
            }
            
            const X_monotone_curve_2 & xc =
                (ps_x == CGAL::ARR_LEFT_BOUNDARY ? 
                 // AFTER DISCONTINUITY
                 (*(event->right_curves_begin()))->last_curve() :
                 // BEFORE_DISCONTINUITY
                 (*(event->left_curves_rbegin()))->last_curve()
                );
            
            Arr_curve_end ind = (ps_x == CGAL::ARR_LEFT_BOUNDARY ?
                                 CGAL::ARR_MIN_END : CGAL::ARR_MAX_END);
            
            const Point_2& key = 
                (ind == CGAL::ARR_MIN_END ?
                 this->m_top_traits->geometry_traits()->
                 construct_min_vertex_2_object()(xc) :
                 this->m_top_traits->geometry_traits()->
                 construct_max_vertex_2_object()(xc));
            
            DVertex * v = m_top_traits->vertex_WE(key);
            
            Vertex_handle vh(v);

            if (v == NULL) {
                vh = m_arr_access.create_boundary_vertex(
                        xc, ind,
                        ps_x, ps_y
                );
            }
            
            event->set_vertex_handle(vh);
            
            return;
        }
        
        if (ps_y != CGAL::ARR_INTERIOR) {
            //std::cout << "HELPER: before y " << ps_y << std::endl;
            CGAL_assertion(ps_y == CGAL::ARR_TOP_BOUNDARY ||
                           ps_y == CGAL::ARR_BOTTOM_BOUNDARY);
            CGAL_assertion(ps_x == CGAL::ARR_INTERIOR);
            CGAL_assertion(event->number_of_left_curves() +
                           event->number_of_right_curves() >= 1);
            
            // TODO isolated point
            
            // added for hole-location
            if (ps_y == CGAL::ARR_TOP_BOUNDARY && 
                m_top_traits->is_identification_NS_empty()) {
                // The list m_subcurves_at_tf contains all subcurves 
                // whose left endpoint lies between the curve of 
                // identification in NS and the current curve incident 
                // to the NS-identification. In case these subcurves 
                // represent holes, these holes should stay in the 
                // "face to the left" that contains the curve 
                // of identification and we should not keep track of 
                // them in order to later move them to another face.
                //std::cout << "delete sc in tf" << std::endl;
                m_subcurves_at_tf.clear();
            }

            const X_monotone_curve_2 & xc =
                (ps_y == CGAL::ARR_BOTTOM_BOUNDARY ? 
                 // AFTER DISCONTINUITY
                 (event->number_of_right_curves() > 0 ? 
                  (*(event->right_curves_begin()))->last_curve() :
                  (*(event->left_curves_rbegin()))->last_curve()) :
                 // BEFORE_DISCONTINUITY
                 (event->number_of_left_curves() > 0 ? 
                  (*(event->left_curves_rbegin()))->last_curve() :
                  (*(event->right_curves_begin()))->last_curve()) 
                );
            
            Arr_curve_end ind =  
                (ps_y == CGAL::ARR_BOTTOM_BOUNDARY ? 
                 (event->number_of_right_curves() > 0 ? 
                  CGAL::ARR_MIN_END : CGAL::ARR_MAX_END) : 
                 (event->number_of_left_curves() > 0 ? 
                  CGAL::ARR_MAX_END : CGAL::ARR_MIN_END)
                );

            const Point_2& key = 
                (ind == CGAL::ARR_MIN_END ?
                 this->m_top_traits->geometry_traits()->
                 construct_min_vertex_2_object()(xc) :
                 this->m_top_traits->geometry_traits()->
                 construct_max_vertex_2_object()(xc));
            
            DVertex * v = m_top_traits->vertex_NS(key);
            
            Vertex_handle vh(v);
            
            if (v == NULL) {
                //std::cout << "HELPER: new y" << std::endl;
                vh = m_arr_access.create_boundary_vertex(xc, ind, ps_x, ps_y);
            }

            event->set_vertex_handle(vh);
            
            return;
        }
    }
    
    /*! A notification invoked when a new subcurve is created. */
    virtual void add_subcurve(Halfedge_handle he, Subcurve * sc) { 
        //std::cout << "HELPER: add_sc" << std::endl;
        //std::cout << "add sc he: " << &(*he) << std::endl;
        // Check whether the halfedge (or its twin) lie on the top face.
        Halfedge_handle     he_on_top_face;
        
        Arr_parameter_space bnd_y_min = 
            this->m_top_traits->geometry_traits()->
            parameter_space_in_y_2_object()(he->curve(), CGAL::ARR_MIN_END);
        Arr_parameter_space bnd_y_max = 
            this->m_top_traits->geometry_traits()->
            parameter_space_in_y_2_object()(he->curve(), CGAL::ARR_MAX_END);
        
        if (bnd_y_min == CGAL::ARR_TOP_BOUNDARY) {
            he_on_top_face = 
                (he->direction() == CGAL::ARR_RIGHT_TO_LEFT ? 
                 he : he->twin());
        } else if (bnd_y_max == CGAL::ARR_TOP_BOUNDARY) {
            he_on_top_face = 
                (he->direction() == CGAL::ARR_LEFT_TO_RIGHT ? 
                 he : he->twin());
        } else {
            return;
        }
        
        // Associate all curve indices of subcurves that lie in the current
        // top face with a halfedge that see this halfedge.
        if (m_he_ind_map_p != NULL) {
            Indices_list& list_ref = (*m_he_ind_map_p)[he_on_top_face];
            typename Indices_list::const_iterator itr;
#if 0
            for (itr = list_ref.begin(); itr != list_ref.end(); ++itr)
            {
                std::cout << "move sc " << *itr << " from tf to " 
                          << he_on_top_face->curve()
                          << (he_on_top_face->direction() 
                          << std::endl;
            }
#endif
            list_ref.splice (list_ref.end(), m_subcurves_at_tf);
        } else {
            //std::cout << "m_sc_tf.clear()" << std::endl;
            m_subcurves_at_tf.clear();
        }
        CGAL_assertion (m_subcurves_at_tf.empty());
        
        return;
    }
    
    /*! Collect a subcurve index that does not see any status-line from below.
     */
    void add_subcurve_in_top_face(unsigned int index)
    {
        //std::cout << "HELPER: add_sc_tf" << std::endl;
        m_subcurves_at_tf.push_back(index);
        return;
    }
    
    /*! A notification invoked before the given event it deallocated. */
    void before_deallocate_event(Event * event) { 
        //std::cout << "HELPER: before deall" << std::endl;
        return; 
    }
    //@} 
    
    /*! Set the map that maps each halfedge to the list of subcurve indices
     * that "see" the halfedge from below.
     */
    void set_halfedge_indices_map(Halfedge_indices_map & table)
    {
        m_he_ind_map_p = &table;
        return;
    }
    
    /*! Determine if we should swap the order of predecessor halfedges when
     * calling insert_at_vertices_ex() .
     */
    bool swap_predecessors(Event * event) const
    {
        return 
            (event->parameter_space_in_x() == CGAL::ARR_INTERIOR &&
             event->parameter_space_in_y() == CGAL::ARR_TOP_BOUNDARY);
    }
    
    /*! Get the current top face. */
    Face_handle top_face() const { 
        //std::cout << "HELPER: top face" << std::endl;
        return m_top_face; 
    }
};

} //namespace CGAL

#endif // CGAL_ARR_TORUS_CONSTRUCTION_HELPER
// EOF
