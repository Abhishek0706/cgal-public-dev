// Copyright (c) 2008  Tel-Aviv University (Israel).
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
// $URL$
// $Id$
// 
//
// Author(s)     : Eric Berberich     <ericb@post.tau.ac.il>


// flags in this file:

#ifndef CGAL_ARR_SINGLE_CELL_2_SUB_NAIVE
#define CGAL_ARR_SINGLE_CELL_2_SUB_NAIVE 0
#endif

#ifndef CGAL_ARR_SINGLE_CELL_2_RI_NAIVE
#define CGAL_ARR_SINGLE_CELL_2_RI_NAIVE 0
#endif

#ifndef CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
#define CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS 0
#endif

#ifndef CGAL_ARR_SINGLE_CELL_2_H
#define CGAL_ARR_SINGLE_CELL_2_H

#include <boost/optional.hpp>
#include <boost/none.hpp>

#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
#include <CGAL/Timer.h>
#endif

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Arr_default_overlay_traits.h>

#include <CGAL/Arr_observer.h>
#include <CGAL/Arrangement_2/Arr_traits_adaptor_2.h>

#include <CGAL/Arr_extended_dcel.h>

// TASK select best point location strategy
#include <CGAL/Arr_naive_point_location.h>

// DOES NOT WORK FOR UNBOUNDED
//#include <CGAL/Arr_simple_point_location.h>
#if !CGAL_USE_ACK_2 
#include <CGAL/Arr_walk_along_line_point_location.h> 
#endif

// required for Constructor of RI_observer
#include <CGAL/Arr_simple_point_location.h>

namespace CGAL {

namespace internal {

template < class Arrangement_2_ >
struct Copy_features_2 {
    
     //! this class template parameter
    typedef Arrangement_2_ Arrangement_2;

    //! geometric traits class
    typedef typename Arrangement_2::Geometry_traits_2 Geometry_traits_2;
    
    //! type of point
    typedef typename Geometry_traits_2::Point_2 Point_2;
    
    //! type of x-monotone curve
    typedef typename Geometry_traits_2::X_monotone_curve_2 
    X_monotone_curve_2;
    
    //! type of curve
    typedef typename Geometry_traits_2::Curve_2 Curve_2;
    
    typedef typename Arrangement_2::Vertex_const_handle 
    Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_const_handle 
    Halfedge_const_handle;
    typedef typename Arrangement_2::Edge_const_iterator Edge_const_iterator;
    typedef typename Arrangement_2::Face_const_handle Face_const_handle;
    
    typedef typename Arrangement_2::Halfedge_around_vertex_const_circulator
    Halfedge_around_vertex_const_circulator;

    typedef typename Arrangement_2::Vertex_handle            Vertex_handle;
    typedef typename Arrangement_2::Halfedge_handle          Halfedge_handle;
    typedef typename Arrangement_2::Face_handle              Face_handle;

    typedef std::vector< Point_2 > Points_container;
    typedef typename Points_container::iterator Points_iterator;
    typedef typename Points_container::const_iterator Points_const_iterator;

    typedef std::vector< X_monotone_curve_2 > Curves_container;
    typedef typename Curves_container::iterator Curves_iterator;
    typedef typename Curves_container::const_iterator Curves_const_iterator;
    
    /*!\brief Copy subarrangement defined by \c cell_handle to \c out
     */
    // FUTURE TODO allow multiple cell_handles!
    void operator()(const CGAL::Object& cell_handle) {
        
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
        _mt_copy_features.start();
#endif
        
        Vertex_const_handle vh;
        Halfedge_const_handle heh;
        Face_const_handle fh;
        if (CGAL::assign(vh, cell_handle)) {
            _m_pts.push_back(vh->point());
        } else if (CGAL::assign(heh, cell_handle)) {
            _m_xcvs.push_back(heh->curve());
        } else {
            CGAL_assertion_code(bool check =)
                CGAL::assign(fh, cell_handle);
            CGAL_assertion(check);
            
            // copy curves of CCBs of face
            typedef typename Arrangement_2::Outer_ccb_const_iterator 
                Outer_ccb_const_iterator;
            
            typedef typename Arrangement_2::Inner_ccb_const_iterator 
                Inner_ccb_const_iterator;
            
            typedef typename Arrangement_2::Ccb_halfedge_const_circulator 
                Ccb_halfedge_const_circulator;
            
            typedef typename Arrangement_2::Isolated_vertex_const_iterator 
                Isolated_vertex_const_iterator;
            
            for (Outer_ccb_const_iterator ocb = fh->outer_ccbs_begin();
                 ocb != fh->outer_ccbs_end(); 
                 ocb++) {
                Ccb_halfedge_const_circulator he = *ocb;
                if (!he->is_fictitious()) {
                    _m_xcvs.push_back(he->curve());
                }
                he++;
                for (; he != *ocb; he++) {
                    if (!he->is_fictitious()) {
                        _m_xcvs.push_back(he->curve());
                    }
                }
            }
            
            for (Inner_ccb_const_iterator icb = fh->inner_ccbs_begin();
                 icb != fh->inner_ccbs_end(); 
                 icb++) {
                Ccb_halfedge_const_circulator he = *icb;
                if (!he->is_fictitious()) {
                    _m_xcvs.push_back(he->curve());
                }
                he++;
                for (; he != *icb; he++) {
                    if (!he->is_fictitious()) {
                        _m_xcvs.push_back(he->curve());
                    }
                }
            }
            
            // copy isolated points of face to cell
            for (Isolated_vertex_const_iterator vt = 
                     fh->isolated_vertices_begin(); 
                 vt != fh->isolated_vertices_end(); vt++) {
                _m_pts.push_back(vt->point());
            }
        }
        
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
        _mt_copy_features.stop();
        
        std::cout << "tCopySub   : " << _mt_copy_features.time() 
                  << " sec" << std::endl;
#endif
    }

    //! beginning of read points
    Points_iterator points_begin() {
        return _m_pts.begin();
    }


    //! past-the-end of read points
    Points_iterator points_end() {
        return _m_pts.end();
    }

     //! beginning of read points (const version)
    Points_const_iterator points_begin() const {
        return _m_pts.begin();
    }


    //! past-the-end of read points (const version)
    Points_const_iterator points_end() const {
        return _m_pts.end();
    }

    //! beginning of read curves
    Curves_iterator curves_begin() {
        return _m_xcvs.begin();
    }


    //! past-the-end of read curves
    Curves_iterator curves_end() {
        return _m_xcvs.end();
    }

     //! beginning of read curves (const version)
    Curves_const_iterator curves_begin() const {
        return _m_xcvs.begin();
    }


    //! past-the-end of read curves (const version)
    Curves_const_iterator curves_end() const {
        return _m_xcvs.end();
    }
    
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
    double total_time() {
        return _mt_copy_features.time();
    }
    
private:
    //! used time
    CGAL::Timer _mt_copy_features;
#endif   
    
    //! read points
    mutable Points_container _m_pts;

    //! read curves
    mutable Curves_container _m_xcvs;
};

/*!\brief
 * Fucntor that extracts subarrangement by given handle
 */
template < class Arrangement_2_ >
struct Sub_arrangement_2 {
    
    //! this class template parameter
    typedef Arrangement_2_ Arrangement_2;

    //! geometric traits class
    typedef typename Arrangement_2::Geometry_traits_2 Geometry_traits_2;
    
    //! type of point
    typedef typename Geometry_traits_2::Point_2 Point_2;
    
    //! type of x-monotone curve
    typedef typename Geometry_traits_2::X_monotone_curve_2 
    X_monotone_curve_2;
    
    //! type of curve
    typedef typename Geometry_traits_2::Curve_2 Curve_2;
    
    typedef typename Arrangement_2::Vertex_const_handle 
    Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_const_handle 
    Halfedge_const_handle;
    typedef typename Arrangement_2::Edge_const_iterator Edge_const_iterator;
    typedef typename Arrangement_2::Face_const_handle Face_const_handle;
    
    typedef typename Arrangement_2::Halfedge_around_vertex_const_circulator
    Halfedge_around_vertex_const_circulator;

    typedef typename Arrangement_2::Vertex_handle            Vertex_handle;
    typedef typename Arrangement_2::Halfedge_handle          Halfedge_handle;
    typedef typename Arrangement_2::Face_handle              Face_handle;
    
    /*!\brief Extracts subarrangement of \c arr defined by \c cell_handle
     */
    // FUTURE TODO allow multiple cell_handles!
    void operator()(Arrangement_2& arr, const CGAL::Object& cell_handle) {
        
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
        _mt_sub.start();
#endif
        
        Vertex_const_handle vh;
        Halfedge_const_handle heh;
        Face_const_handle fh;
        if (CGAL::assign(vh, cell_handle)) {
            
            // delete all but vh
            for (Halfedge_const_handle eit = 
                     arr.halfedges_begin();
                 eit != arr.halfedges_end(); eit++, eit++) {
                arr.remove_edge(
                        arr.non_const_handle(eit)
                );
            }
            
            for (Vertex_const_handle vit = 
                     arr.vertices_begin();
                 vit != arr.vertices_end(); vit++) {
                if (vit == vh) {
                    continue;
                }
                // else
                arr.remove_isolated_vertex(
                        arr.non_const_handle(vit)
                );
            }
            
        } else if (CGAL::assign(heh, cell_handle)) {

            // delete all but heh
            for (Halfedge_const_handle eit = 
                     arr.halfedges_begin();
                 eit != arr.halfedges_end(); eit++, eit++) {
                if (eit == heh || eit->twin() == heh) {
                    continue;
                }
                // else
                arr.remove_edge(
                        arr.non_const_handle(eit)
                );
            }
            
            for (Vertex_const_handle vit = 
                     arr.vertices_begin();
                 vit != arr.vertices_end(); vit++) {
                if (!vit->is_isolated()) {
                    continue;
                }
                // else
                arr.remove_isolated_vertex(
                        arr.non_const_handle(vit)
                );
            }
            
        } else {
            CGAL_assertion_code(bool check =)
                CGAL::assign(fh, cell_handle);
            CGAL_assertion(check);
            
            for (Halfedge_const_handle eit = arr.halfedges_begin();
                 eit != arr.halfedges_end(); eit++, eit++) {
                if (eit->face() == fh || eit->twin()->face() == fh) {
                    continue;
                }
                // else
                //std::cout << "DELETE edge: " << &(*eit) << std::endl;
                arr.remove_edge(arr.non_const_handle(eit));
            }
            
            for (Vertex_const_handle vit = arr.vertices_begin();
                 vit != arr.vertices_end(); vit++) {
                if (!vit->is_isolated()) {
                    continue;
                }
                if (vit->face() == fh) {
                    continue;
                }
                // else
                arr.remove_isolated_vertex(arr.non_const_handle(vit));
            }
        }
        
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
        _mt_sub.stop();
        
        std::cout << "tSub   : " << _mt_sub.time() 
                  << " sec" << std::endl;
#endif
    }

#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
    double total_time() {
        return _mt_sub.time();
    }

private:
    CGAL::Timer _mt_sub;
#endif
};

/*!\brief
 * class that observes a city-arrangement
 * 
 * Keeps eye on face- and edge-splits
 * and updates internal structures wrt to given query point
 */
template < class Arrangement_2_ >
struct RI_observer : CGAL::Arr_observer< Arrangement_2_ > {
    
    //! this class template parameter
    typedef Arrangement_2_ Arrangement_2;

    //! base class
    typedef CGAL::Arr_observer< Arrangement_2 > Base;
    
    //! geometric traits class
    typedef typename Arrangement_2::Geometry_traits_2 Geometry_traits_2;
    
    //! traits adaptor
    typedef Arr_traits_basic_adaptor_2< Geometry_traits_2 > Traits_adaptor_2;
    
    //! type of point
    typedef typename Geometry_traits_2::Point_2 Point_2;
    
    //! type of x-monotone curve
    typedef typename Geometry_traits_2::X_monotone_curve_2 
    X_monotone_curve_2;
    
    //! type of curve
    typedef typename Geometry_traits_2::Curve_2 Curve_2;
    
    typedef typename Arrangement_2::Vertex_const_handle 
    Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_const_handle 
    Halfedge_const_handle;
    typedef typename Arrangement_2::Edge_const_iterator Edge_const_iterator;
    typedef typename Arrangement_2::Face_const_handle Face_const_handle;
    
    typedef typename Arrangement_2::Halfedge_around_vertex_const_circulator
    Halfedge_around_vertex_const_circulator;

    typedef typename Arrangement_2::Vertex_handle            Vertex_handle;
    typedef typename Arrangement_2::Halfedge_handle          Halfedge_handle;
    typedef typename Arrangement_2::Face_handle              Face_handle;
    
    //!\name Constructors
    //!@{
    
    // FUTURE TODO allow multiple points

    //! default constructor from arr and point
    RI_observer(Arrangement_2& arr, const Point_2& point) :
        Base(arr),
        _m_point(point),
        _m_check_split_edges(false),
        _m_face_split(false) {
        
        // determine initial _m_edge_handle
        // TODO avoid ray_shoot up
        CGAL::Arr_simple_point_location< Arrangement_2 > 
            ray(*(Base::arrangement()));
        CGAL::Object obj = ray.ray_shoot_up(_m_point);
        Vertex_const_handle vh;
        
        if (CGAL::assign(_m_halfedge_handle, obj)) {
            
            if (_m_halfedge_handle->direction() != CGAL::ARR_RIGHT_TO_LEFT) {
                _m_halfedge_handle = _m_halfedge_handle->twin();
            }
            CGAL_assertion(
                    _m_halfedge_handle->direction() == CGAL::ARR_RIGHT_TO_LEFT
            );
            
            _m_cell_handle = CGAL::make_object(_m_halfedge_handle->face());
            
        } else if (CGAL::assign(vh, obj)) {
            
            // TODO correct assign
            std::cerr << "has been assigned point" << std::endl;
            
        }  else {
            
            // TODO does not work with unbounded that are bordered by
            //      infinite curves
            Face_const_handle fh;
            CGAL_assertion_code(bool check =)
                CGAL::assign(fh, obj);
            CGAL_assertion(check);
            
            Vertex_const_handle 
                vtr(Base::arrangement()->topology_traits()->
                    top_right_vertex());
            
            Halfedge_around_vertex_const_circulator heh  = 
                vtr->incident_halfedges();
            heh++;
            
            CGAL_assertion_code(
                    Vertex_const_handle 
                    vtl(Base::arrangement()->topology_traits()->
                        top_left_vertex())
            );
            CGAL_assertion(heh->source() == vtl);
            
            _m_halfedge_handle = heh->twin();
            CGAL_assertion(
                    _m_halfedge_handle->direction() == CGAL::ARR_RIGHT_TO_LEFT
            );
            _m_cell_handle = CGAL::make_object(_m_halfedge_handle->face());
        }

#if 0
        if (_m_halfedge_handle->is_fictitious()) {
            std::cout << "Initial curve: fict" << std::endl;
        } else {
            std::cout << "Initial curve: " << _m_halfedge_handle->curve()
                      << std::endl;
        }
#endif
    }

    //!@}
    
    //!\name Notifications
    //!@{

    /*!
     * Notification after the creation of a new edge.
     * \param e A handle to one of the twin halfedges that were created.
     */
    virtual void after_create_edge (Halfedge_handle e)
    {
        typename Geometry_traits_2::Is_vertical_2 is_vertical =
            Base::arrangement()->geometry_traits()->is_vertical_2_object();
        typename Traits_adaptor_2::Is_in_x_range_2 is_in_x_range =
            static_cast< Traits_adaptor_2* >
            (Base::arrangement()->geometry_traits())->is_in_x_range_2_object();
        typename Geometry_traits_2::Compare_y_at_x_2 compare_y_at_x =
            Base::arrangement()->geometry_traits()->compare_y_at_x_2_object();
        
        CGAL_assertion(!e->is_fictitious());
        
        X_monotone_curve_2 new_cv = e->curve();
        
        //std::cout << "after create edge" << std::endl;
        
        if (!is_vertical(new_cv)) {
            // TODO symbolic pertubation for endpoints!
            if (is_in_x_range(new_cv, _m_point)) {
                CGAL::Comparison_result res = 
                    compare_y_at_x(_m_point, new_cv);
                
                if (res == CGAL::SMALLER) {
                    
                    bool change = false;
                    
                    if (_m_halfedge_handle->is_fictitious()) {
                        change = true;
                        //std::cout << "CHANGE 1 (old fict)" << std::endl;
                    } else {
                        X_monotone_curve_2 old_cv = 
                            _m_halfedge_handle->curve();
                        
                        //std::cout << "old_cv: " << old_cv << std::endl;
                        //std::cout << "new_cv: " << new_cv << std::endl;

                        typename Traits_adaptor_2::Compare_y_position_2 
                            compare_y_pos =
                            static_cast< const Traits_adaptor_2* >
                            (Base::arrangement()->geometry_traits())->
                            compare_y_position_2_object();
                        
                        if (is_in_x_range(new_cv, old_cv)) {
                            if (compare_y_pos(new_cv, old_cv) ==
                                CGAL::SMALLER) {
                                change = true;
                                //std::cout << "CHANGE 2" << std::endl;
                            }
                        } else {
                            typename Geometry_traits_2::Compare_x_2 compare_x =
                                Base::arrangement()->geometry_traits()->
                                compare_x_2_object();
                            typename Geometry_traits_2::Construct_min_vertex_2 
                                construct_min =
                                Base::arrangement()->geometry_traits()->
                                construct_min_vertex_2_object();
                            typename Geometry_traits_2::Construct_max_vertex_2 
                                construct_max =
                                Base::arrangement()->geometry_traits()->
                                construct_max_vertex_2_object();
                            
                            Point_2 min = construct_min(new_cv);
                            if (compare_x(min, _m_point) == CGAL::EQUAL) {
                                change = (compare_y_at_x(min, old_cv) 
                                          == CGAL::SMALLER);
                                if (change) {
                                    //std::cout << "CHANGE 3" << std::endl;
                                }
                            } else {
                                Point_2 max = construct_max(new_cv);
                                CGAL_assertion(
                                        compare_x(max, _m_point) == CGAL::EQUAL
                                );
                                change = (compare_y_at_x(max, old_cv)
                                          == CGAL::SMALLER);
                                if (change) {
                                    //std::cout << "CHANGE 4" << std::endl;
                                }
                            }
                        }
                    }

                    // finally
                    if (change) {
                        // the new edge replaces the old one
                        // we only have to ensure the correct order
                        _m_halfedge_handle = 
                            ((e->direction() == CGAL::ARR_RIGHT_TO_LEFT) ?
                             e : e->twin());
                        
                        CGAL_assertion(
                                _m_halfedge_handle->direction() == 
                                CGAL::ARR_RIGHT_TO_LEFT
                        );
                        
                        _m_cell_handle = 
                            CGAL::make_object(_m_halfedge_handle->face());

                        //std::cout << "HE1 set to cv: " 
                        //          << _m_halfedge_handle->curve() 
                        //          << std::endl;
                    }
                } else if (res == CGAL::EQUAL) {
                    // TODO on curve!!!!
                }
            }
        } else {
            // TODO what if e->curve() is vertical
        }

        if (_m_face_split) {
            CGAL_assertion(
                    _m_halfedge_handle->direction() == CGAL::ARR_RIGHT_TO_LEFT
            );
            _m_cell_handle = CGAL::make_object(_m_halfedge_handle->face());
            
            
#if !CGAL_ARR_SINGLE_CELL_2_SUB_NAIVE
            // what TODO with this simplication - it leads - naively used - 
            // to seg-faults and inconsistencies in zoning
            Sub_arrangement_2< Arrangement_2 > subarr;
            subarr(*(Base::arrangement()), _m_cell_handle);
#endif
            _m_face_split = false;
        }
    }

    /*!
     * Notification after a face was split.
     * \param f A handle to the face we have just split.
     * \param new_f A handle to the new face that has been created.
     * \param is_hole Whether the new face forms a hole inside f.
     */
    virtual void after_split_face (Face_handle /* f */,
                                   Face_handle /* new_f */,
                                   bool /* is_hole */)
    {
        //std::cout << "after_split_face" << std::endl;
        _m_face_split = true;
    }
    
    /*!
     * Notification before the splitting of an edge into two.
     * \param e A handle to one of the existing halfedges.
     * \param v A vertex representing the split point.
     * \param c1 The x-monotone curve to be associated with the first edge.
     * \param c2 The x-monotone curve to be associated with the second edge.
     */
    virtual void before_split_edge (Halfedge_handle e,
                                    Vertex_handle /* v */,
                                    const X_monotone_curve_2& /* c1 */,
                                    const X_monotone_curve_2& /* c2 */)
    {
        Halfedge_handle curr = 
            Base::arrangement()->non_const_handle(_m_halfedge_handle);
//         if (curr->is_fictitious()) {
//             std::cout << "curr fict";
//         } else {
//             std::cout << "curr: " << curr->curve() << std::endl;
//         }
//        std::cout << std::endl;
        if (e == curr || e->twin() == curr) {
            //std::cout << "It's maybe required UPDATE _m_halfedge_handle"
            //          << std::endl;
            _m_check_split_edges = true;
        }
        
    }
    
    /*!
     * Notification after an edge was split.
     * \param e1 A handle to one of the twin halfedges forming the first edge.
     * \param e2 A handle to one of the twin halfedges forming the second edge.
     */
    virtual void after_split_edge (Halfedge_handle e1,
                                   Halfedge_handle e2)
    {
        typename Geometry_traits_2::Is_vertical_2 is_vertical =
            Base::arrangement()->geometry_traits()->is_vertical_2_object();
        typename Traits_adaptor_2::Is_in_x_range_2 is_in_x_range =
            static_cast< const Traits_adaptor_2* >
            (Base::arrangement()->geometry_traits())->is_in_x_range_2_object();
        
        typename Geometry_traits_2::Compare_y_at_x_2 compare_y_at_x =
            Base::arrangement()->geometry_traits()->compare_y_at_x_2_object();
        
        X_monotone_curve_2 cv1 = e1->curve();
        X_monotone_curve_2 cv2 = e2->curve();
        
        if (_m_check_split_edges) {
            //std::cout << "pt : " << _m_point << std::endl;
            //std::cout << "cv1: " << cv1 << std::endl;
            //std::cout << "cv2: " << cv2 << std::endl;
            
            // Remark:
            // as e1 and e2 originate from _m_halfedge_handle
            // we only have to check x-ranges ...
            if (!is_vertical(cv1)) {
                CGAL_assertion(!is_vertical(cv2));
                // TODO symbolic pertubation for endpoints!
                if (is_in_x_range(cv1, _m_point)) {
                    _m_halfedge_handle = 
                        ((e1->direction() == CGAL::ARR_RIGHT_TO_LEFT) ?
                         e1 : e1->twin());

                    CGAL_assertion(
                            _m_halfedge_handle->direction() == 
                            CGAL::ARR_RIGHT_TO_LEFT
                    );
                    
                    _m_cell_handle = 
                        CGAL::make_object(_m_halfedge_handle->face());
                    
                    //std::cout << "HE3 set to cv: " 
                    //          << _m_halfedge_handle->curve() << std::endl;

                } else {
                    CGAL_assertion(is_in_x_range(cv2, _m_point));
                    _m_halfedge_handle = 
                        ((e2->direction() == CGAL::ARR_RIGHT_TO_LEFT) ?
                         e2 : e2->twin());
                    
                    CGAL_assertion(
                            _m_halfedge_handle->direction() == 
                            CGAL::ARR_RIGHT_TO_LEFT
                    );
                    
                    _m_cell_handle = 
                        CGAL::make_object(_m_halfedge_handle->face());
                    
                    //std::cout << "HE4 set to cv: " 
                    //          << _m_halfedge_handle->curve() << std::endl;
                    
                }
            } else {
                // or something in vertical case
                // TODO what if e->curve() is vertical
            }
        }
        
        // finally reset trigger
        _m_check_split_edges = false;
    }
    
    /*!
     * Notification before the splitting of a fictitious edge into two.
     * \param e A handle to one of the existing halfedges.
     * \param v A vertex representing the unbounded split point.
     */
    virtual void before_split_fictitious_edge (Halfedge_handle /* e */,
                                               Vertex_handle /* v */)
    {
        // TODO is required for update of fictitious _m_halfedge_handle
    }
    
    /*!
     * Notification after a fictitious edge was split.
     * \param e1 A handle to one of the twin halfedges forming the first edge.
     * \param e2 A handle to one of the twin halfedges forming the second edge.
     */
    virtual void after_split_fictitious_edge (Halfedge_handle /* e1 */,
                                              Halfedge_handle /* e2 */)
    {
        // TODO is required for update of fictitious _m_halfedge_handle
    }

    //!@}
    
   //! returns cell_handle to cell of given point
    CGAL::Object cell_handle() {
        return _m_cell_handle;
    }

private:
    //! query point
    Point_2 _m_point;
    
    //! halfedge "immediately above point"
    mutable Halfedge_const_handle _m_halfedge_handle;
    
    //! cell handle to compute
    CGAL::Object _m_cell_handle;

    // helper for split edge
    //! if true has to choose among two split edges
    bool _m_check_split_edges;

    //! if true face has split
    bool _m_face_split;
};



template < class BoolArrangement_2 >
class RB_overlay_traits : 
        public CGAL::Arr_default_overlay_traits< BoolArrangement_2 > {
    
public:
    //! this class template parameter
    typedef BoolArrangement_2 Arrangement_2;

    //! geometric traits class
    typedef typename Arrangement_2::Geometry_traits_2 Geometry_traits_2;
    
    //! type of point
    typedef typename Geometry_traits_2::Point_2 Point_2;
    
    //! type of x-monotone curve
    typedef typename Geometry_traits_2::X_monotone_curve_2 
    X_monotone_curve_2;
    
    //! type of curve
    typedef typename Geometry_traits_2::Curve_2 Curve_2;
    
    typedef typename Arrangement_2::Vertex_const_handle 
    Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_const_handle 
    Halfedge_const_handle;
    typedef typename Arrangement_2::Edge_const_iterator Edge_const_iterator;
    typedef typename Arrangement_2::Face_const_handle Face_const_handle;
    
    typedef typename Arrangement_2::Halfedge_around_vertex_const_circulator
    Halfedge_around_vertex_const_circulator;

    typedef typename Arrangement_2::Vertex_handle            Vertex_handle;
    typedef typename Arrangement_2::Halfedge_handle          Halfedge_handle;
    typedef typename Arrangement_2::Face_handle              Face_handle;
    
    typedef std::vector< Point_2 > Points_container;
    typedef typename Points_container::iterator Points_iterator;
    typedef typename Points_container::const_iterator Points_const_iterator;

    typedef std::vector< X_monotone_curve_2 > Curves_container;
    typedef typename Curves_container::iterator Curves_iterator;
    typedef typename Curves_container::const_iterator Curves_const_iterator;

    /*!
     * Create a face f that matches the overlapping region between f1 and f2.
     */
    virtual void create_face (Face_const_handle f1,
                              Face_const_handle f2,
                              Face_const_handle f) const
    {
        if (f1->data() && f2->data()) {
            
            Copy_features_2< Arrangement_2 > copy_features;
            copy_features(CGAL::make_object(f));

            std::copy(copy_features.curves_begin(), copy_features.curves_end(),
                      std::back_inserter(_m_xcvs));

            std::copy(copy_features.points_begin(), copy_features.points_end(),
                      std::back_inserter(_m_pts));
            
            return;
        }
    }


  //! beginning of read points
    Points_iterator points_begin() {
        return _m_pts.begin();
    }


    //! past-the-end of read points
    Points_iterator points_end() {
        return _m_pts.end();
    }

     //! beginning of read points (const version)
    Points_const_iterator points_begin() const {
        return _m_pts.begin();
    }


    //! past-the-end of read points (const version)
    Points_const_iterator points_end() const {
        return _m_pts.end();
    }

    //! beginning of read curves
    Curves_iterator curves_begin() {
        return _m_xcvs.begin();
    }


    //! past-the-end of read curves
    Curves_iterator curves_end() {
        return _m_xcvs.end();
    }

     //! beginning of read curves (const version)
    Curves_const_iterator curves_begin() const {
        return _m_xcvs.begin();
    }


    //! past-the-end of read curves (const version)
    Curves_const_iterator curves_end() const {
        return _m_xcvs.end();
    }

private:
    
    mutable Points_container _m_pts;
    mutable Curves_container _m_xcvs;

};

/*! \class Functor to compute single cell of point */
template < class Arrangement_2_ >
class Construct_single_cell_2 {
    
public:

    //! this instance's first template parameter
    typedef Arrangement_2_ Arrangement_2;

    //! the class itself
    typedef Construct_single_cell_2< Arrangement_2 > Self;

    //! geometric traits class
    typedef typename Arrangement_2::Geometry_traits_2 Geometry_traits_2;

    //! type of point
    typedef typename Geometry_traits_2::Point_2 Point_2;

    //! type of x-monotone curve
    typedef typename Geometry_traits_2::X_monotone_curve_2 X_monotone_curve_2;

    //! type of curve
    typedef typename Geometry_traits_2::Curve_2 Curve_2;


    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_const_handle 
    Halfedge_const_handle;
    typedef typename Arrangement_2::Edge_const_iterator Edge_const_iterator;
    typedef typename Arrangement_2::Face_const_handle Face_const_handle;

    // TASK select best point location strategy
    //! type of point location strategy
    
#if CGAL_USE_ACK_2 
    typedef CGAL::Arr_naive_point_location< Arrangement_2 > Point_location;
#else
    // DO NOT WORK FOR UNBOUNDED!
    //typedef CGAL::Arr_simple_point_location< Arrangement_2 > Point_location;
    typedef Arr_walk_along_line_point_location< Arrangement_2 > Point_location;
#endif
    
    //Point_location;

    //!\name Constructors
    //!@{
    
    template < class InputIterator >
    Construct_single_cell_2(InputIterator begin, InputIterator end) :
        _m_cell_handle_pl(boost::none),
        _m_cell_handle_ri(boost::none),
        _m_cell_handle_rbo(boost::none) {
        
        // TODO check that iteratortype is CGAL::Object
        _m_objects.reserve(std::distance(begin, end));
        
#if !NDEBUG 
        std::cout << "Created Construct_single_cell_2 instance for "
                  << std::distance(begin, end) << " input objects."
                  << std::endl;
#endif

        for (InputIterator it = begin; it != end; it++) {
            _m_objects.push_back(*it);
            
            X_monotone_curve_2 curr_xcurve;
            Point_2 curr_point;
            Curve_2 curr_curve;
            
            if (CGAL::assign(curr_curve, *it)) {
                std::list< CGAL::Object > tmp;
#if CGAL_USE_ACK_2
                typename Geometry_traits_2::Make_x_monotone_2
                    make_x_monotone =
                    Geometry_traits_2::instance().make_x_monotone_2_object();
#else
                typename Geometry_traits_2::Make_x_monotone_2
                    make_x_monotone;
#endif
                make_x_monotone(curr_curve, std::back_inserter(tmp));
                
                for (typename std::list< CGAL::Object >::const_iterator 
                         iit = tmp.begin();
                     iit != tmp.end(); iit++) {
                    if (CGAL::assign(curr_xcurve,*iit)) {
                        _m_xcvs.push_back(curr_xcurve);
                    } else {
                        CGAL_assertion_code(bool check = )
                            CGAL::assign(curr_point,*iit);
                        CGAL_assertion(check);
                        _m_pts.push_back(curr_point);
                    }
                }
            } else if (CGAL::assign(curr_xcurve,*it)) {
                _m_xcvs.push_back(curr_xcurve);
            } else {
                CGAL_assertion_code(bool check = )
                    CGAL::assign(curr_point,*it);
                CGAL_assertion(check);
                _m_pts.push_back(curr_point);
            }
        }
    }
    
    //!@}
    
public:
    
    //!\name cell localizations
    //!@{

    //! returns the cell using point location
    CGAL::Object cell_pl(const Point_2& pt) const {
        if (!_m_cell_handle_pl) {
#if !NDEBUG
            std::cout << "Computing cell with POINT-LOCATION-method ... " 
                      << std::flush;
#endif
            // compute full arr
            this->_m_full_arr = Arrangement_2();
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            _mt_full_arr.start();
#endif
            CGAL::insert_empty(*_m_full_arr,
                               _m_xcvs.begin(), _m_xcvs.end(),
                               _m_pts.begin(), _m_pts.end()
            );
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            _mt_full_arr.stop();
#endif
            
#if !NDEBUG
            std::cout << "The full_arr sizes:" << std::endl
                      << "   V = " << _m_full_arr->number_of_vertices()
                      << ",  E = " << _m_full_arr->number_of_edges() 
                      << ",  F = " << _m_full_arr->number_of_faces() 
                      << std::endl;
#endif 
            // locate point
            Point_location pl(*_m_full_arr);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            _mt_pl.start();
#endif
            _m_cell_handle_pl = pl.locate(pt);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            _mt_pl.stop();
#endif
            
#if !NDEBUG
            std::cout << "done."   << std::endl << std::endl;
#endif
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            std::cout << "tFullArr: " << _mt_full_arr.time() 
                      << " sec" << std::endl;
            std::cout << "tPL     : " << _mt_pl.time() 
                      << " sec" << std::endl;
#endif
        }
        return *_m_cell_handle_pl;
    }

public:
    //! returns the cell using random incremental
    CGAL::Object cell_ri(const Point_2& pt) const {
        if (!_m_cell_handle_ri) {
#if !NDEBUG
            std::cout << "Computing cell with RANDOM_INCREMENTAL-method ... " 
                      << std::flush;
#endif
            _m_arr_city = Arrangement_2();
            
            CGAL::Object cell_handle;            
            
            // let's start with all points
            CGAL::non_intersecting_insert_empty(
                    *_m_arr_city,
                    _m_xcvs.begin(), _m_xcvs.begin(), // NO CURVE
                    _m_pts.begin(), _m_pts.end()
            );
            
            Point_location pl(*_m_arr_city);
            cell_handle = pl.locate(pt);
            
            Face_const_handle fh;
            if (!CGAL::assign(fh, cell_handle)) {
                // simple case
                _m_cell_handle_ri = cell_handle; // found point
                
            } else {
                
                // random shuffle
                // TODO use better random values
                std::random_shuffle(_m_xcvs.begin(), _m_xcvs.end());
                
                Face_const_handle fh = _m_arr_city->faces_begin();
                cell_handle = CGAL::make_object(fh);
                
#if !CGAL_ARR_SINGLE_CELL_2_SUB_NAIVE
                Sub_arrangement_2< Arrangement_2 > subarr;
#endif
                
                // real RI-case
                for (typename 
                         std::vector< X_monotone_curve_2 >::const_iterator 
                         cit = _m_xcvs.begin(); cit != _m_xcvs.end(); cit++) {

#if !CGAL_ARR_SINGLE_CELL_2_RI_NAIVE
                    internal::RI_observer< Arrangement_2 > 
                        obs((*_m_arr_city), pt);
#endif
                    //std::cout << "Insert: " << *cit << std::endl;
                    // add *cit using zone
                    CGAL::insert(*_m_arr_city, *cit);

#if CGAL_ARR_SINGLE_CELL_2_RI_NAIVE
                    // make point location
                    cell_handle = pl.locate(pt);
#else
                    // point location via observing arrangement
                    cell_handle = obs.cell_handle();
#endif
                    
                    // sub!
#if CGAL_ARR_SINGLE_CELL_2_SUB_NAIVE
                    Arrangement_2 new_city;
                    cell_arr(cell_handle, new_city);
                    _m_arr_city = new_city;
#else
                    subarr(*_m_arr_city, cell_handle);
#endif
                }

                _m_cell_handle_ri = cell_handle;
            }
            
#if !NDEBUG
            std::cout << "done."   << std::endl;
#endif
        }
        return *_m_cell_handle_ri;
    }
    
    CGAL::Object cell_rbo_naive(const Point_2& pt) const {
        if (!_m_cell_handle_rbo) {
#if !NDEBUG
            std::cout << "Computing cell for " 
                      << (_m_xcvs.size() + _m_pts.size())
                      << " input objects " 
                      << "with NAIVE-RED-BLUE-OVERLAY-method ... " 
                      << std::flush;
#endif

            if (_m_xcvs.size() + _m_pts.size() <= 4) {
                
#if !NDEBUG
                std::cout << "Anchor" << std::endl;
#endif
                
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_anchor.start();
#endif
                CGAL::Object cell_handle = cell_pl(pt);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_anchor.stop();
#endif
                
                _m_cell_handle_rbo = cell_handle;
                
            } else {
                
#if !NDEBUG
                std::cout << "Red-blue split" << std::endl;
#endif
                // permute input
                // TODO use better random values
                std::random_shuffle(_m_xcvs.begin(), _m_xcvs.end());
                std::random_shuffle(_m_pts.begin(), _m_pts.end());

                // split input into two sets
                std::vector< X_monotone_curve_2 > xcvs[2];
                typename 
                    std::vector< X_monotone_curve_2 >::iterator 
                    xcvs_mid =
                    _m_xcvs.begin();
                std::advance(xcvs_mid, (_m_xcvs.size() / 2));
                xcvs[0].reserve(std::distance(_m_xcvs.begin(), xcvs_mid));
                std::copy(_m_xcvs.begin(), xcvs_mid, 
                          std::back_inserter(xcvs[0]));
                xcvs[1].reserve(std::distance(xcvs_mid, _m_xcvs.end()));
                std::copy(xcvs_mid, _m_xcvs.end(), 
                          std::back_inserter(xcvs[1]));
                
                std::vector< Point_2 > pts[2];
                typename std::vector< Point_2 >::iterator pts_mid =
                    _m_pts.begin();
                std::advance(pts_mid, (_m_pts.size() / 2));
                pts[0].reserve(std::distance(_m_pts.begin(), pts_mid));
                std::copy(_m_pts.begin(), pts_mid, std::back_inserter(pts[0]));
                pts[1].reserve(std::distance(pts_mid, _m_pts.end()));
                std::copy(pts_mid, _m_pts.end(), std::back_inserter(pts[1]));
                
                CGAL::Object cell_handle[2];
                
                Arrangement_2 cell[2];
                
                for (int i = 0; i < 2; i++) {
                    
                    std::list< CGAL::Object > objects;
                    for (typename 
                             std::vector< X_monotone_curve_2 >::const_iterator
                             it = xcvs[i].begin(); it != xcvs[i].end(); it++) {
                        objects.push_back(CGAL::make_object(*it));
                    }
                    for (typename std::vector< Point_2 >::const_iterator
                             it = pts[i].begin(); it != pts[i].end(); it++) {
                        objects.push_back(CGAL::make_object(*it));
                    }
                    
                    Self recursive(objects.begin(), objects.end());
                    
                    cell_handle[i] = recursive.cell_rbo_naive(pt);
                    
                    // Remark: Cannot use sub_arr here, 
                    //         as no access to overlaid arrangement
                    cell_arr(cell_handle[i], cell[i]);
                }
                
#if !NDEBUG
                std::cout << "Start overlay ... " << std::flush;
#endif

#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_overlay.start();
#endif
                CGAL::Arr_default_overlay_traits< Arrangement_2 > ovltraits;

                Arrangement_2 arr_purple;

                CGAL::overlay(cell[0], cell[1], 
                              arr_purple, ovltraits);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_overlay.stop();
#endif
                
                _m_arr_purple = arr_purple;
                
#if !NDEBUG
                std::cout << "done." << std::endl;
#endif
                
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_pl.start();
#endif
                Point_location pl_purple(*_m_arr_purple);
                CGAL::Object cell_handle_pl_purple = pl_purple.locate(pt);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_pl.stop();
#endif   
                
                _m_cell_handle_rbo = cell_handle_pl_purple;
            }
#if !NDEBUG
            std::cout << "done." << std::endl;
            std::cout << std::endl;
#endif
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            std::cout << "tRecAnchor : " << _mt_rec_anchor.time() 
                      << " sec" << std::endl;
            std::cout << "tRecOverlay: " << _mt_rec_overlay.time() 
                      << " sec" << std::endl;
            std::cout << "tPLs       : " << _mt_pl.time() 
                      << " sec" << std::endl;
#endif
        }
        return *_m_cell_handle_rbo;
    }


    //! returns the cell using red-blue overlay
    CGAL::Object cell_rbo(const Point_2& pt) const {
        if (!_m_cell_handle_rbo) {
            
            typedef 
                CGAL::Arr_extended_dcel< Geometry_traits_2, bool, bool, bool > 
                Bool_dcel;
            
            typedef CGAL::Arrangement_2< Geometry_traits_2, Bool_dcel >
                Bool_arrangement_2;
            
#if !NDEBUG
            std::cout << "Computing cell for " 
                      << (_m_xcvs.size() + _m_pts.size())
                      << " input objects " 
                      << "with RED-BLUE-OVERLAY-method ... " 
                      << std::flush;
#endif
            if (_m_xcvs.size() + _m_pts.size() <= 4) {
                
#if !NDEBUG
                std::cout << "Anchor" << std::endl;
#endif
                
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_anchor.start();
#endif
                CGAL::Object cell_handle = cell_pl(pt);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_anchor.stop();
#endif
                
                _m_cell_handle_rbo = cell_handle;
                
            } else {
                
#if !NDEBUG
                std::cout << "Red-blue split" << std::endl;
#endif
                // permute input
                // TODO use better random values
                std::random_shuffle(_m_xcvs.begin(), _m_xcvs.end());
                std::random_shuffle(_m_pts.begin(), _m_pts.end());
                
                // split input into two sets
                std::vector< X_monotone_curve_2 > xcvs[2];
                typename 
                    std::vector< X_monotone_curve_2 >::iterator 
                    xcvs_mid =
                    _m_xcvs.begin();
                std::advance(xcvs_mid, (_m_xcvs.size() / 2));
                xcvs[0].reserve(std::distance(_m_xcvs.begin(), xcvs_mid));
                std::copy(_m_xcvs.begin(), xcvs_mid, 
                          std::back_inserter(xcvs[0]));
                xcvs[1].reserve(std::distance(xcvs_mid, _m_xcvs.end()));
                std::copy(xcvs_mid, _m_xcvs.end(), 
                          std::back_inserter(xcvs[1]));
                
                std::vector< Point_2 > pts[2];
                typename std::vector< Point_2 >::iterator pts_mid =
                    _m_pts.begin();
                std::advance(pts_mid, (_m_pts.size() / 2));
                pts[0].reserve(std::distance(_m_pts.begin(), pts_mid));
                std::copy(_m_pts.begin(), pts_mid, std::back_inserter(pts[0]));
                pts[1].reserve(std::distance(pts_mid, _m_pts.end()));
                std::copy(pts_mid, _m_pts.end(), std::back_inserter(pts[1]));
                
                CGAL::Object cell_handle[2];
                
                Bool_arrangement_2 cell[2];
                
                for (int i = 0; i < 2; i++) {
                    
                    std::list< CGAL::Object > objects;
                    for (typename 
                             std::vector< X_monotone_curve_2 >::const_iterator
                             it = xcvs[i].begin(); it != xcvs[i].end(); it++) {
                        objects.push_back(CGAL::make_object(*it));
                    }
                    for (typename std::vector< Point_2 >::const_iterator
                             it = pts[i].begin(); it != pts[i].end(); it++) {
                        objects.push_back(CGAL::make_object(*it));
                    }
                    
                    Construct_single_cell_2< Bool_arrangement_2 > 
                        recursive(objects.begin(), objects.end());
                    
                    cell_handle[i] = recursive.cell_rbo(pt);
                    
                    // Remark: Cannot use sub_arr here, 
                    //         as no access to overlaid arrangement
                    recursive.cell_arr(cell_handle[i], cell[i]);
                    
                    typedef typename  Bool_arrangement_2::Face_const_handle
                        BFace_const_handle;
                    
                    typedef Arr_walk_along_line_point_location< 
                    Bool_arrangement_2 > BPoint_location;

                    BPoint_location pl(cell[i]);
                    
                    cell_handle[i] = pl.locate(pt);
                    
                    BFace_const_handle fh;

                    if (CGAL::assign(fh, cell_handle[i])) {
                        
                        // TODO right now we ONLY mark faces
                        for (BFace_const_handle fit = cell[i].faces_begin();
                             fit != cell[i].faces_end(); fit++) {
                            if (fit == fh) {
                                cell[i].non_const_handle(fit)->set_data(true);
                            } else {
                                cell[i].non_const_handle(fit)->set_data(false);
                            }
                        }
                    }
                }
#if !NDEBUG
                std::cout << "Start overlay ... " << std::flush;
#endif

#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_overlay.start();
#endif
                internal::RB_overlay_traits< Bool_arrangement_2 > 
                    ovltraits;

                Bool_arrangement_2 arr_purple_tmp;
                
                // TODO replace overlay by SL-Visitor that directly constructs
                // arr_purple (i.e., without arr_purple_tmp)
                // curves + isolated points
                CGAL::overlay(cell[0], cell[1], 
                              arr_purple_tmp, ovltraits);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_rec_overlay.stop();
#endif
#if !NDEBUG
                std::cout << "done." << std::endl;
#endif
                
                _m_arr_purple = Arrangement_2();
                
                // TODO use CGAL::non_intersecting_insert_empty(
                //      requires: non-equal curves!
                CGAL::insert_empty(
                        *_m_arr_purple,
                        ovltraits.curves_begin(), 
                        ovltraits.curves_end(),
                        ovltraits.points_begin(), 
                        ovltraits.points_end()
            );
                

#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_pl.start();
#endif
                Point_location pl_purple(*_m_arr_purple);
                CGAL::Object cell_handle_pl_purple = pl_purple.locate(pt);
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
                _mt_pl.stop();
#endif   
                
                Sub_arrangement_2< Arrangement_2 > subarr;
                
                subarr(*_m_arr_purple, cell_handle_pl_purple);

                _m_cell_handle_rbo = cell_handle_pl_purple;
            }
#if !NDEBUG
            std::cout << "done." << std::endl;
            std::cout << std::endl;
#endif
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
            std::cout << "tRecAnchor : " << _mt_rec_anchor.time() 
                      << " sec" << std::endl;
            std::cout << "tRecOverlay: " << _mt_rec_overlay.time() 
                      << " sec" << std::endl;
            std::cout << "tPLs       : " << _mt_pl.time() 
                      << " sec" << std::endl;
#endif
        }
        return *_m_cell_handle_rbo;
    }
    
    //!@}

    //!\name Helpers
    //!@{
        
    /*!\brief
     * converts a cell-handle (face, edge, vertex) into its induced arrangement      */
        // FUTURE TODO allow multiple cell_handles!
        void cell_arr(CGAL::Object cell_handle, Arrangement_2& cell) const {
            
            Copy_features_2< Arrangement_2 > copy_features;
            
            copy_features(cell_handle);
            
            // TODO use CGAL::non_intersecting_insert_empty(
            //      requires: non-equal curves!
            CGAL::insert_empty(
                    cell,
                    copy_features.curves_begin(), 
                    copy_features.curves_end(),
                    copy_features.points_begin(), 
                    copy_features.points_end()
            );
        }

private:

    //////////////////////////////////////////////////////////////////////////
    // members

    //! input objects
    std::vector< CGAL::Object > _m_objects;
    
    //! input curves
    mutable std::vector< X_monotone_curve_2 > _m_xcvs;

    //! input points
    mutable std::vector< Point_2 > _m_pts;
    
    //! the cell
    mutable boost::optional< CGAL::Object > _m_cell_handle_pl; 
    mutable boost::optional< CGAL::Object > _m_cell_handle_ri; 
    mutable boost::optional< CGAL::Object > _m_cell_handle_rbo; 

    // helper for pl
    mutable boost::optional< Arrangement_2 > _m_full_arr; 

    // helper for ri
    mutable boost::optional< Arrangement_2 > _m_arr_city;

    // helper for rbo_naive
    mutable boost::optional< Arrangement_2 > _m_arr_purple; 

    // timers
#if CGAL_ARR_SINGLE_CELL_2_SHOW_TIMINGS
    mutable CGAL::Timer _mt_full_arr;
    mutable CGAL::Timer _mt_pl;
    mutable CGAL::Timer _mt_cell;

    mutable CGAL::Timer _mt_rec_anchor;
    mutable CGAL::Timer _mt_rec_overlay;
#endif
};

} // namespace internal

/*!
 * Construct the single cell containing a point using point location
 * \param point The reference point
 * \param begin An iterator for the first input object defining the full 
 *              arrangement
 * \param end A past-the-end iterator for the input objects defining the
 *            full arrangement
 * \param cell Output: The cell of Arr(begin,end) containing point as 
 *                     arrangement
 * \pre The value-type of InputIterator is CGAL::Object which can be passed to
 *      GeoTraits_2::Make_x_monotone_2()
 */
template < typename InputIterator, typename GeoTraits_2 >
CGAL::Object single_cell_pl_2(
        typename GeoTraits_2::Point_2 point,
        InputIterator begin, InputIterator end,
        CGAL::Arrangement_2< GeoTraits_2 >& cell) {
    
    typedef GeoTraits_2                                       Geo_traits_2;
    typedef CGAL::Arrangement_2< Geo_traits_2 >               Arrangement_2;
    
    typedef internal::Construct_single_cell_2< Arrangement_2 >   
        Construct_single_cell_2;
    
    Construct_single_cell_2 single_cell(
            begin, end
    );

    CGAL::Object cell_handle = single_cell.cell_pl(point);
    // TODO avoid to reconstruct arrangement!
    single_cell.cell_arr(cell_handle, cell);
    return cell_handle;
}

/*!
 * Construct the single cell containing a point using random incremental 
 * \param point The reference point
 * \param begin An iterator for the first input object defining the full 
 *              arrangement
 * \param end A past-the-end iterator for the input objects defining the
 *            full arrangement
 * \param cell Output: The cell of Arr(begin,end) containing point as 
 *                     arrangement
 * \pre The value-type of InputIterator is CGAL::Object which can be passed to
 *      GeoTraits_2::Make_x_monotone_2()
 */
template < typename InputIterator, typename GeoTraits_2 >
CGAL::Object single_cell_ri_2(
        typename GeoTraits_2::Point_2 point,
        InputIterator begin, InputIterator end,
        CGAL::Arrangement_2< GeoTraits_2 >& cell) {
    
    typedef GeoTraits_2                                       Geo_traits_2;
    typedef CGAL::Arrangement_2< Geo_traits_2 >               Arrangement_2;
    
    typedef internal::Construct_single_cell_2< Arrangement_2 >   
        Construct_single_cell_2;
    
    Construct_single_cell_2 single_cell(
            begin, end
    );

    CGAL::Object cell_handle = single_cell.cell_ri(point);
    // TODO avoid to reconstruct arrangement
    single_cell.cell_arr(cell_handle, cell);
    return cell_handle;
 }


/*!
 * Construct the single cell containing a point using naive red-blue overlay
 * \param point The reference point
 * \param begin An iterator for the first input object defining the full 
 *              arrangement
 * \param end A past-the-end iterator for the input objects defining the
 *            full arrangement
 * \param cell Output: The cell of Arr(begin,end) containing point as 
 *                     arrangement
 * \pre The value-type of InputIterator is CGAL::Object which can be passed to
 *      GeoTraits_2::Make_x_monotone_2()
 */
template < typename InputIterator, typename GeoTraits_2 >
CGAL::Object single_cell_rbo_naive_2(
        typename GeoTraits_2::Point_2 point,
        InputIterator begin, InputIterator end,
        CGAL::Arrangement_2< GeoTraits_2 >& cell) {
    
    typedef GeoTraits_2                                       Geo_traits_2;
    typedef CGAL::Arrangement_2< Geo_traits_2 >               Arrangement_2;
    
    typedef internal::Construct_single_cell_2< Arrangement_2 >   
        Construct_single_cell_2;
    
    Construct_single_cell_2 single_cell(
            begin, end
    );
    
    CGAL::Object cell_handle = single_cell.cell_rbo_naive(point);
    // TODO avoid to reconstruct arrangement!
    single_cell.cell_arr(cell_handle, cell);
    return cell_handle;
}


/*!
 * Construct the single cell containing a point using red-blue overlay
 * \param point The reference point
 * \param begin An iterator for the first input object defining the full 
 *              arrangement
 * \param end A past-the-end iterator for the input objects defining the
 *            full arrangement
 * \param cell Output: The cell of Arr(begin,end) containing point as 
 *                     arrangement
 * \pre The value-type of InputIterator is CGAL::Object which can be passed to
 *      GeoTraits_2::Make_x_monotone_2()
 */
template < typename InputIterator, typename GeoTraits_2 >
CGAL::Object single_cell_rbo_2(
        typename GeoTraits_2::Point_2 point,
        InputIterator begin, InputIterator end,
        CGAL::Arrangement_2< GeoTraits_2 >& cell) {
    
    typedef GeoTraits_2                                       Geo_traits_2;
    typedef CGAL::Arrangement_2< Geo_traits_2 >               Arrangement_2;
    
    typedef internal::Construct_single_cell_2< Arrangement_2 >   
        Construct_single_cell_2;
    
    Construct_single_cell_2 single_cell(
            begin, end
    );
    
    CGAL::Object cell_handle = single_cell.cell_rbo(point);
    // TODO avoid to reconstruct arrangement!
    single_cell.cell_arr(cell_handle, cell);
    return cell_handle;
}


} //namespace CGAL

#endif // CGAL_ARR_SINGLE_CELL_2_H
// EOF

