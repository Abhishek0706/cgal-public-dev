// Copyright (c) 2012  Tel-Aviv University (Israel).
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
// Author(s)     : Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_TYPEDEFS_H
#define CGAL_TYPEDEFS_H

#include <CGAL/General_polygon_set_2.h>
#include <iostream>
//#include <CGAL/Gps_traits_2.h>
//#include <CGAL/Arr_segment_traits_2.h>
//#include <QT5/Gps_segment_traits_2_apurva.h>
//#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
//#include <CGAL/Qt/PolygonWithHolesGraphicsItem.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Arr_segment_traits_2.h>


//#include <CGAL/Gps_circle_segment_traits_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel        Kernel;//Gps_linear_kernel;
//shift it to where it is needed
/*
typedef CGAL::Gps_segment_traits_2_apurva<Kernel>           Linear_traits;
typedef Linear_traits::Curve_2                        Linear_curve;
typedef Linear_traits::X_monotone_curve_2             Linear_X_monotone_curve;
//typedef Linear_traits::Point_2                                Linear_point ;
typedef Linear_traits::Polygon_2                              Linear_polygon;
typedef CGAL::General_polygon_with_holes_2<Linear_polygon>    Linear_polygon_with_holes;
*/

//typedef CGAL::Gps_segment_traits_2<Kernel>
typedef CGAL::Gps_traits_2<CGAL::Arr_segment_traits_2<Kernel>> Linear_traits;
typedef Linear_traits::General_polygon_with_holes_2  Linear_polygon_with_holes;
//<Linear_polygon>

//check out
typedef CGAL::General_polygon_set_2<Linear_traits>            Linear_polygon_set;
//manage it
typedef std::vector<Linear_polygon_with_holes>  Linear_region_source_container;

// Circlular polygons

//typedef CGAL::Cartesian<Coord_type> Gps_circular_kernel;

//typedef Linear_kernel                             Circular_Linear_kernel;//check out for future may generate a bug               
     
typedef Kernel::Point_2                    Circular_Linear_point;
typedef CGAL::Polygon_2<Kernel>            Circular_Linear_polygon;
typedef CGAL::Polygon_with_holes_2<Kernel> Circular_Linear_polygon_with_holes;

typedef CGAL::Gps_circle_segment_traits_2<Kernel>  Circular_traits;
typedef Circular_traits::Curve_2                   Circular_curve;
typedef Circular_traits::X_monotone_curve_2        Circular_X_monotone_curve;
typedef Circular_traits::Point_2                   Circular_point;
typedef Circular_traits::Polygon_2                 Circular_polygon;
typedef CGAL::General_polygon_with_holes_2<Circular_polygon>   Circular_polygon_with_holes;
typedef CGAL::General_polygon_set_2<Circular_traits>           Circular_polygon_set;

typedef std::vector<Circular_polygon_with_holes>  Circular_region_source_container ;

#endif // CGAL_TYPEDEFS_H



/*
typedef struct Iterator_and_polygons
{
public:    
    typedef Linear_traits::Curve_const_iterator        Curve_const_iterator;
    typedef CGAL::General_polygon_set_2<Linear_traits> Polygons;
} Linear_polygon_set;
*/


/*
#ifdef CGAL_USE_GMP

  typedef CGAL::Gmpq                     Base_nt;

#else

  typedef CGAL::Quotient<CGAL::MP_Float> Base_nt;

#endif
*/
//typedef Kernel::FT Base_nt;
//typedef CGAL::Lazy_exact_nt<Base_nt> Coord_type;

//Linear polygons

//typedef CGAL::Simple_cartesian<Coord_type>                       Linear_kernel;


/*
typedef std::vector<Linear_kernel::Point_2>                  Linear_Container;
typedef CGAL::Arr_segment_traits_2<Linear_kernel>            ArrSegmentTraits;
struct Gps_linear_kernel : public CGAL::Cartesian<Coord_type> {};
/*
struct Gps_linear_kernel : public ,
                           public ,
                           public {};
*/
/*
typedef CGAL::Gps_segment_traits_2<CGAL::Exact_predicates_exact_constructions_kernel,
                                   Linear_Container,
                                   ArrSegmentTraits>         Linear_traits;*/
