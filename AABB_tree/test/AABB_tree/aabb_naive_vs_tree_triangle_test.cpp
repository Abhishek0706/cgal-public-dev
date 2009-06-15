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
// $URL$
// $Id$
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description :
//
//******************************************************************************

#include <fstream>
#include <iostream>

#include <CGAL/Timer.h>

#include <CGAL/AABB_intersections.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_polyhedron_triangle_primitive.h>

#include "AABB_test_util.h"


template<class K, class Tree, class Polyhedron, Primitive_type Type>
void test_impl(Tree& tree, Polyhedron& p, const double duration)
{
  typedef Tree_vs_naive<Tree, Polyhedron, K, Type> Tester;
  Tester tester(tree, p);

  tester.test_all_intersection_methods(duration);
}

int main(void)
{
  std::cout << "AABB naive vs tree tests" << std::endl;

  const double duration = 0.2;
  test_kernels<TRIANGLE>("./data/cube.off",duration);
  test_kernels<TRIANGLE>("./data/coverrear.off",duration);
  test_kernels<TRIANGLE>("./data/nested_spheres.off",duration);
  test_kernels<TRIANGLE>("./data/finger.off",duration);
  return 0;
}
