// Copyright (c) 2006,2007,2009,2010,2011,2014 Tel-Aviv University (Israel).
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
//
//
// Author(s)     : Tali Zvi <talizvi@post.tau.ac.il>
//                 Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Ron Wein <wein@post.tau.ac.il>
//                 Efi Fogel <efif@post.tau.ac.il>
//                 Eric Berberich <eric.berberich@cgal.org>

#ifndef CGAL_SWEEP_LINE_FUNCTORS_H
#define CGAL_SWEEP_LINE_FUNCTORS_H

/*! \file
 * Comparison functors used by the sweep-line algorithm.
 */

#include <CGAL/assertions.h>
#include <CGAL/enum.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/Arrangement_2/Arr_traits_adaptor_2.h>

namespace CGAL {

/*! \class
 * A functor used to compare events and event points in an xy-lexicographic
 * order. Used to maintain the order of the event queue (the X-structure)
 * in the sweep-line algorithm.
 */
template <class Traits_, class Event_>
class Compare_events
{
public:

  typedef Traits_                                   Traits_2;
  typedef Event_                                    Event;

  typedef typename Traits_2::Point_2                Point_2;
  typedef typename Traits_2::X_monotone_curve_2     X_monotone_curve_2;

  // should be ok, as Traits_2 is supposed to be the adaptor
  typedef typename Traits_2::Left_side_category          Left_side_category;
  typedef typename Traits_2::Bottom_side_category        Bottom_side_category;
  typedef typename Traits_2::Top_side_category           Top_side_category;
  typedef typename Traits_2::Right_side_category         Right_side_category;

private:

  // Data members:
  const Traits_2 * m_traits;                // The geometric-traits object.

  Arr_parameter_space  m_ps_x;              // Storing curve information when
  Arr_parameter_space  m_ps_y;              // comparing a curve end with
  Arr_curve_end        m_ind;               // boundary conditions.

public:

  /*! Constructor. */
  Compare_events (const Traits_2 * traits) :
    m_traits (traits)
  {}

  //!\name Function call operators
  //!@{

  /*! Compare two existing events.
   * This operator is called by the multiset assertions only in
   * debug mode (to verify that event was inserted at the right place).
   */
  Comparison_result operator()(const Event* e1, const Event* e2) const
  {
#if 0
    CGAL::set_pretty_mode(std::cout);
    std::cout << "\n FUNCTOR e1-e2" << std::endl;
    std::cout << "e1: " << e1 << std::endl;
    std::cout << "ps_x1: " << e1->parameter_space_in_x() << std::endl;
    std::cout << "ps_y1: " << e1->parameter_space_in_y() << std::endl;
    std::cout << "e2: " << e2 << std::endl;
    std::cout << "ps_x2: " << e2->parameter_space_in_x() << std::endl;
    std::cout << "ps_y2: " << e2->parameter_space_in_y() << std::endl;
#endif

    const bool is_isolated1 = e1->is_isolated();
    const bool is_isolated2 = e2->is_isolated();

    if (is_isolated1) {
      const Point_2& pt1 = e1->point();
      Arr_parameter_space ps_x1 = e1->parameter_space_in_x();
      Arr_parameter_space ps_y1 = e1->parameter_space_in_y();
      if (is_isolated2) {
        const Point_2& pt2 = e2->point();
        Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
        Arr_parameter_space ps_y2 = e2->parameter_space_in_y();
        return _compare_points(pt1, ps_x1, ps_y1, pt2, ps_x2, ps_y2);
      } else {
        const X_monotone_curve_2& cv2 = e2->curve(); // TODO curve() might not be unique
        Arr_curve_end ind2 = _curve_end(e2);
        Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
        Arr_parameter_space ps_y2 = e2->parameter_space_in_y();
        return _compare_point_curve_end(pt1, ps_x1, ps_x2, cv2, ind2, ps_x2, ps_y2);
      }
    } else {
      const X_monotone_curve_2& cv1 = e1->curve(); // TODO curve() might not be unique
        Arr_curve_end ind1 = _curve_end(e1);
        Arr_parameter_space ps_x1 = e1->parameter_space_in_x();
        Arr_parameter_space ps_y1 = e1->parameter_space_in_y();
      if (is_isolated2) {
        const Point_2& pt2 = e2->point();
        Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
        Arr_parameter_space ps_y2 = e2->parameter_space_in_y();
        return CGAL::opposite(_compare_point_curve_end(pt2, ps_x2, ps_y2, cv1, ind1, ps_x1, ps_y1));
      } else {
        const X_monotone_curve_2& cv2 = e2->curve(); // TODO curve() might not be unique
        Arr_curve_end ind2 = _curve_end(e2);
        Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
        Arr_parameter_space ps_y2 = e2->parameter_space_in_y();
        return _compare_curve_ends(cv1, ind1, ps_x1, ps_y1, cv2, ind2, ps_x2, ps_y2);
      }
    }
  }

  /*! Compare an isolated point, which should be inserted into the event queue,
   * with an existing event (which might lie on the boundary).
   * Note that its boundary conditions must be set beforehand
   * using set_parameter_space_in_x/y().
   */
  Comparison_result operator() (const Point_2& pt1, const Event* e2) const
  {
    // REMARK: use m_ps_x and m_ps_y as set from outside! do not rely on pt here anymore
    Arr_parameter_space ps_x1 = m_ps_x;
    Arr_parameter_space ps_y1 = m_ps_y;
    Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
    Arr_parameter_space ps_y2 = e2->parameter_space_in_y();

    if (e2->is_isolated()) {
      const Point_2& pt2 = e2->point();
      return _compare_points(pt1, ps_x1, ps_y1, pt2, ps_x2, ps_y2);
    }

    // else
    const X_monotone_curve_2& cv2 = e2->curve(); // TODO curve() might not be unique
    Arr_curve_end ind2 = _curve_end(e2);
    return _compare_point_curve_end(pt1, ps_x1, ps_y1, cv2, ind2, ps_x2, ps_y2);
  }

  /*! Compare a curve end, which should be inserted into the event queue,
   * with an existing event (which might lie on the boundary).
   * Note that the ind of the curve end as well as its boundary conditions
   * must be set beforehand using set_ind() and set_parameter_space_in_x/y().
   */
  Comparison_result operator() (const X_monotone_curve_2& cv1,
                                const Event* e2) const
  {
    // REMARK: use m_ind, m_ps_x and m_ps_y as set from outside! do not rely on cv here anymore
    Arr_curve_end ind1 = m_ind;
    Arr_parameter_space ps_x1 = m_ps_x;
    Arr_parameter_space ps_y1 = m_ps_y;
    Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
    Arr_parameter_space ps_y2 = e2->parameter_space_in_y();

    if (e2->is_isolated()) {
      const Point_2& pt2 = e2->point();
      return CGAL::opposite(_compare_point_curve_end(pt2, ps_x2, ps_y2, cv1, ind1, ps_x1, ps_y1));
    }

    // else
    const X_monotone_curve_2& cv2 = e2->curve(); // TODO curve() might not be unique
    Arr_curve_end ind2 = _curve_end(e2);
    return _compare_curve_ends(cv1, ind1, ps_x1, ps_y1, cv2, ind2, ps_x2, ps_y2);
  }

  //!@}

  //!\name Set the boundary conditions of a curve end we are about to compare.
  //!@{


  //! sets ps_x
  void set_parameter_space_in_x (Arr_parameter_space ps_x)
  {
    m_ps_x = ps_x;
  }

  //! sets ps_y
  void set_parameter_space_in_y (Arr_parameter_space ps_y)
  {
    m_ps_y = ps_y;
  }

  //! sets ind for a curve
  void set_ind (Arr_curve_end ind)
  {
    m_ind = ind;
  }

  //!@}

private:

  // The following three functions basically adapt the case distincting of this table
  // (in particular for the 'easy cases' that consists only of a single word and for which the
  // following common comparison can be used:

    // e1 left      e2 left      if a point exists (pt in signature, or vertical case): compare_y_on_boundary(pt1, pt2) REMARK: obviously points on the left boundary are allowed, otherwise: compare_y_curve_ends(ce1, ce2, ind)
    // e1 left      e2 bottom    SMALLER
    // e1 left      e2 interior  SMALLER
    // e1 left      e2 top       SMALLER
    // e1 left      e2 right     SMALLER

    // e1 bottom    e2 left      LARGER
    // e1 bottom    e2 bottom    one of the three signatures of compare_x_on_boundary
    // e1 bottom    e2 interior  one of the three signatures of compare_x_on_boundary
    // e1 bottom    e2 top       one of the three signatures of compare_x_on_boundary
    // e1 bottom    e2 right     SMALLER

    // e1 interior  e2 left      LARGER
    // e1 interior  e2 bottom    one of the three signatures of compare_x_on_boundary
    // e1 interior  e2 interior  compare_xy(pt, pt)
    // e1 interior  e2 top       one of the three signatures of compare_x_on_boundary
    // e1 interior  e2 right     SMALLER

    // e1 top       e2 left      LARGER
    // e1 top       e2 bottom    one of the three signatures of compare_x_on_boundary
    // e1 top       e2 interior  one of the three signatures of compare_x_on_boundary
    // e1 top       e2 top       one of the three signatures of compare_x_on_boundary
    // e1 top       e2 right     SMALLER

    // e1 right     e2 left      LARGER
    // e1 right     e2 bottom    LARGER
    // e1 right     e2 interior  LARGER
    // e1 right     e2 top       LARGER
    // e1 right     e2 right     if a point exists (pt in signature, or vertical case): compare_y_on_boundary(pt1, pt2) REMARK: obviously points on the left boundary are allowed, otherwise: compare_y_curve_ends(ce1, ce2, ind)

 //! called when no further geometric predicate is needed
  //! \pre ps_x1 != ps_x2
  Comparison_result
  _compare_sides(Arr_parameter_space ps_x1,
                 Arr_parameter_space ps_y1,
                 Arr_parameter_space ps_x2,
                 Arr_parameter_space ps_y2) const {

    CGAL_precondition(ps_x1 != ps_x2);
    // only the 14 simple ps_x different cases remain:
    // 1L2B, 1L2I, 1L2T, 1L2R,
    // 1B2L, 1B2R,
    // 1I2L, 1I2R,
    // 1T2L, 1T2R
    // 1R2L, 1R2B, 1R2I, 1R2T
    if (ps_x1 == ARR_LEFT_BOUNDARY) {
      //std::cout << "res4 SMALLER" << std::endl;
      return (SMALLER);
    }
    if (ps_x1 == ARR_RIGHT_BOUNDARY) {
      //std::cout << "res5 LARGER" << std::endl;
      return (LARGER);
    }
    // ps_x1 == ARR_INTERIOR != ps_x2
    if (ps_x2 == ARR_LEFT_BOUNDARY) {
      //std::cout << "res6 LARGER" << std::endl;
      return (LARGER);
    }
    // ps_x2 == ARR_RIGHT_BOUNDARY
    //std::cout << "res7 SMALLER" << std::endl;
    return (SMALLER);
  }

  /*! Compare two given isolated points.
   *
   * \param pt1 The first point.
   * \param ps_x1 The boundary condition of the first point in x.
   * \param ps_y1 The boundary condition of the first point in y.
   * \param pt2 The second point.
   * \param ps_x2 The boundary condition of the second point in x.
   * \param ps_y2 The boundary condition of the second point in y.
   * \return The comparison result of the two points.
   */
  Comparison_result
  _compare_points(const Point_2& pt1,
                  Arr_parameter_space ps_x1,
                  Arr_parameter_space ps_y1,
                  const Point_2& pt2,
                  Arr_parameter_space ps_x2,
                  Arr_parameter_space ps_y2) const {
#if 0
    CGAL::set_pretty_mode(std::cout);
    std::cout << std::endl << "FUNCTOR pt-pt" << std::endl;
    std::cout << "pt1  : " << pt1 << std::endl;
    std::cout << "ps_x1: " << ps_x1 << std::endl;
    std::cout << "ps_y1: " << ps_y1 << std::endl;
    std::cout << "pt2  : " << pt2 << std::endl;
    std::cout << "ps_x2: " << ps_x2 << std::endl;
    std::cout << "ps_y2: " << ps_y2 << std::endl;
#endif

    if (ps_x1 == ps_x2) {
      // same x-partition
      if (ps_x1 != ARR_INTERIOR) {
        // 1L2L, 1R2R
        // e2->point() is accessible, as pt is a point on the SAME left/right side
        Comparison_result res = m_traits->compare_y_on_boundary_2_object() (pt1, pt2);
        //std::cout << "res1 " << res << std::endl;
        return res;
      }
      // else both are x-interior
      if ((ps_y1 == ARR_INTERIOR) && (ps_y2 == ARR_INTERIOR)) {
        // both are y-interior, too 1I2I:
        Comparison_result res = m_traits->compare_xy_2_object() (pt1, pt2);
        //std::cout << "res2 " << res << std::endl;
        return res;
      } else {
        // at least one of pt1 or pt22 lies on a boundary
        Comparison_result res = m_traits->compare_x_on_boundary_2_object() (pt1, pt2);
        //std::cout << "res3 " << res << std::endl;
        return res;
      }
    }

    return _compare_sides(ps_x1, ps_y1, ps_x2, ps_y2);
  }


  /*! Compares a given isolated point with a curve-end.
   *
   * \param pt1 The point.
   * \param ps_x1 The boundary condition of the point in x.
   * \param ps_y1 The boundary condition of the point in y.
   * \param cv2 The curve.
   * \param ind2 The curve end.
   * \param ps_x2 The boundary condition of the curve end in x.
   * \param ps_y2 The boundary condition of the curve end in y.
   * \return The comparison result of the point with the curve end.
   */
  Comparison_result
  _compare_point_curve_end(const Point_2& pt1,
                           Arr_parameter_space ps_x1,
                           Arr_parameter_space ps_y1,
                           const X_monotone_curve_2& cv2,
                           Arr_curve_end ind2,
                           Arr_parameter_space ps_x2,
                           Arr_parameter_space ps_y2) const {

#if 0
    CGAL::set_pretty_mode(std::cout);
    std::cout << std::endl << "FUNCTOR pt1-cv2" << std::endl;
    std::cout << "pt1  : " << pt1 << std::endl;
    std::cout << "ps_x1: " << ps_x1 << std::endl;
    std::cout << "ps_y1: " << ps_y1 << std::endl;
    std::cout << "cv2 " << (m_traits->is_vertical_2_object()(cv2) ? "|" :  " ") << ": " << cv2 << ", " << ind2 << std::endl;
    std::cout << "ps_x2: " << ps_x2 << std::endl;
    std::cout << "ps_y2: " << ps_y2 << std::endl;
#endif

    if (ps_x1 == ps_x2) {
      // same x-partition

      // second point must be accessible, as pt1 is an isolated point on the SAME left/right side or INTERIOR
      const Point_2& pt2 = m_traits->construct_vertex_at_curve_end_2_object()(cv2, ind2);

      if (ps_x1 != ARR_INTERIOR) {
        // 1L2L, 1R2R
        Comparison_result res = m_traits->compare_y_on_boundary_2_object() (pt1, pt2);
        //std::cout << "res1 " << res << std::endl;
        return res;
      }
      // else both are x-interior
      if ((ps_y1 == ARR_INTERIOR) && (ps_y2 == ARR_INTERIOR)) {
        // both are y-interior, too 1I2I:
        Comparison_result res = m_traits->compare_xy_2_object()(pt1, pt2);
        //std::cout << "res2 " << res << std::endl;
        return res;
      } else {
        // at least one of pt1 or cv2 lies on a boundary
        Comparison_result res = m_traits->compare_x_point_curve_end_2_object() (pt1, cv2, ind2);
        //std::cout << "res3 " << res << std::endl;
        return res;
      }
    }

    return _compare_sides(ps_x1, ps_y1, ps_x2, ps_y2);
  }

   /*! Compares two given curve-ends.
   * \param cv1 The first curve.
   * \param ind1 The first curve end.
   * \param ps_x1 The boundary condition of the first curve end in x.
   * \param ps_y1 The boundary condition of the first curve end in y.
   * \param cv2 The second curve.
   * \param ind2 The second curve end.
   * \param ps_x2 The boundary condition of the second curve end in x.
   * \param ps_y2 The boundary condition of the second curve end in y.
   * \return The comparison result of the two curve ends.
   */
  Comparison_result
  _compare_curve_ends(const X_monotone_curve_2& cv1,
                      Arr_curve_end ind1,
                      Arr_parameter_space ps_x1,
                      Arr_parameter_space ps_y1,
                      const X_monotone_curve_2& cv2,
                      Arr_curve_end ind2,
                      Arr_parameter_space ps_x2,
                      Arr_parameter_space ps_y2) const
  {
#if 0
    CGAL::set_pretty_mode(std::cout);
    std::cout << std::endl << "FUNCTOR cv1-cv2"<< std::endl;
    std::cout << "cv1 " << (m_traits->is_vertical_2_object()(cv1) ? "|" :  " ") << ": " << cv1 << ", " << ind1 << std::endl;
    std::cout << "ps_x1: " << ps_x1 << std::endl;
    std::cout << "ps_y1: " << ps_y1 << std::endl;
    std::cout << "cv2 " << (m_traits->is_vertical_2_object()(cv2) ? "|" :  " ") << ": " << cv2 << ", " << ind2 << std::endl;
    std::cout << "ps_x2: " << ps_x2 << std::endl;
    std::cout << "ps_y2: " << ps_y2 << std::endl;
#endif

    if (ps_x1 == ps_x2) {
      // same x-partition

      // second point must be accessible, as pt1 is an isolated point on the SAME left/right side or INTERIOR
      if (ps_x1 != ARR_INTERIOR) {
        // 1L2L, 1R2R

        if (m_traits->is_vertical_2_object()(cv1) || m_traits->is_vertical_2_object()(cv2)) {
          const Point_2& pt1 = m_traits->construct_vertex_at_curve_end_2_object()(cv1, ind1);
          const Point_2& pt2 = m_traits->construct_vertex_at_curve_end_2_object()(cv2, ind2);
          Comparison_result res = m_traits->compare_y_on_boundary_2_object() (pt1, pt2);
          //std::cout << "res1V: " << res << std::endl;
          return res;
        }
        CGAL_assertion(ind1 == ind2);
        Comparison_result res = m_traits->compare_y_curve_ends_2_object() (cv1, cv2, ind1);
        //std::cout << "res1 " << res << std::endl;
        return res;
      }
      // else both are x-interior
      if ((ps_y1 == ARR_INTERIOR) && (ps_y2 == ARR_INTERIOR)) {
        // both are y-interior, too 1I2I:
        const Point_2& pt1 = m_traits->construct_vertex_at_curve_end_2_object()(cv1, ind1);
        const Point_2& pt2 = m_traits->construct_vertex_at_curve_end_2_object()(cv2, ind2);
        Comparison_result res = m_traits->compare_xy_2_object()(pt1, pt2);
        //std::cout << "res2 " << res << std::endl;
        return res;
      } else {
        // at least one of pt1 or cv2 lies on a boundary
        Comparison_result res = m_traits->compare_x_curve_ends_2_object() (cv1, ind1, cv2, ind2);
        //std::cout << "res3 " << res << std::endl;
        return res;
      }
    }

    return _compare_sides(ps_x1, ps_y1, ps_x2, ps_y2);
  }

  /*! Determine if the given event represents a left or a right curve end. */
  inline Arr_curve_end _curve_end (const Event* e) const
  {
    // e must be an open event here, and thus exactly one curve is incident
    // TODO see TODO above CGAL_precondition(!e->is_closed());
    return (e->has_left_curves()) ?
      ((e->is_right_end()) ? ARR_MAX_END : ARR_MIN_END) :
      ((e->is_left_end()) ? ARR_MIN_END : ARR_MAX_END);
  }
};

// Forward decleration:
template <class Traits, class Subcurve>
class Sweep_line_event;

/*! \class
 * A functor used to compare curves and curve endpoints by their vertical
 * y-order. Used to maintain the order of the status line (the Y-structure)
 * in the sweep-line algorithm.
 */
template <class Traits_, class Subcurve_>
class Curve_comparer
{
public:

  typedef Traits_                                        Traits_2;
  typedef Subcurve_                                      Subcurve;
  typedef Arr_traits_basic_adaptor_2<Traits_2>           Traits_adaptor_2;

  typedef typename Traits_adaptor_2::Point_2 Point_2;
  typedef typename Traits_adaptor_2::X_monotone_curve_2  X_monotone_curve_2;
  typedef Sweep_line_event<Traits_2, Subcurve>           Event;

private:

  const Traits_adaptor_2 * m_traits;    // A geometric-traits object.
  Event            **m_curr_event;      // Points to the current event point.

public:

  /*! Constructor. */
  template <class Sweep_event>
  Curve_comparer (const Traits_adaptor_2 * t, Sweep_event** e_ptr) :
    m_traits(t),
    m_curr_event(reinterpret_cast<Event**>(e_ptr))
  {}

  /*!
   * Compare the vertical position of two subcurves in the status line.
   * This operator is called only in debug mode.
   */
  Comparison_result operator()(const Subcurve *c1, const Subcurve *c2) const
  {
    // In case to two curves are right curves at the same event, compare
    // to the right of the event point.
    if (std::find((*m_curr_event)->right_curves_begin(),
                  (*m_curr_event)->right_curves_end(),
                  c1) != (*m_curr_event)->right_curves_end() &&
        std::find((*m_curr_event)->right_curves_begin(),
                  (*m_curr_event)->right_curves_end(),
                  c2) != (*m_curr_event)->right_curves_end())
    {
      return (m_traits->compare_y_at_x_right_2_object()
              (c1->last_curve(), c2->last_curve(), (*m_curr_event)->point()));
    }

    Arr_parameter_space ps_x1 =
      m_traits->parameter_space_in_x_2_object()(c1->last_curve(), ARR_MIN_END);
    Arr_parameter_space ps_y1 =
      m_traits->parameter_space_in_y_2_object()(c1->last_curve(), ARR_MIN_END);

    if ((ps_x1 == ARR_INTERIOR) && (ps_y1 == ARR_INTERIOR))
    {
      // The first curve has a valid left endpoint. Compare the y-position
      // of this endpoint to the second subcurve.
      return m_traits->compare_y_at_x_2_object()
        (m_traits->construct_min_vertex_2_object()(c1->last_curve()),
         c2->last_curve());
    }

    // We use the fact that the two curves are interior disjoint. As c2 is
    // already in the status line, then if c1 left end has a negative boundary
    // condition it obviously above it.
    CGAL_assertion (ps_x1 != ARR_RIGHT_BOUNDARY);

    if (ps_x1 == ARR_LEFT_BOUNDARY)
      return (LARGER);

    // For similar reasons, if c1 begins on the bottom boundary it is below
    // c2, if it is on the top boundary it is above it.
    CGAL_assertion (ps_y1 != ARR_INTERIOR);
    return (ps_y1 == ARR_BOTTOM_BOUNDARY) ? SMALLER : LARGER;
  }

  /*!
   * Compare the relative y-order of the given point and the given subcurve.
   */
  Comparison_result operator() (const Point_2& pt, const Subcurve *sc) const
  {
    return (m_traits->compare_y_at_x_2_object()(pt, sc->last_curve()));
  }

};

} //namespace CGAL

#endif
