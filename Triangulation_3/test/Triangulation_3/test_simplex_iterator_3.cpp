// Copyright (c) 1998  INRIA Sophia-Antipolis (France).
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
// Author(s)     : Jane Tournois


#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_segment_traverser_3.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/Random.h>

// Define the kernel.
typedef CGAL::Exact_predicates_exact_constructions_kernel     Kernel;
typedef Kernel::Point_3                                         Point_3;
typedef Kernel::Vector_3                                        Vector_3;

// Define the structure.
typedef CGAL::Delaunay_triangulation_3< Kernel >    DT;

typedef DT::Vertex_handle                           Vertex_handle;
typedef DT::Cell_handle                             Cell_handle;
typedef DT::Edge                                    Edge;
typedef DT::Facet                                   Facet;

typedef CGAL::Triangulation_segment_simplex_iterator_3<DT>      Simplex_traverser;


void test_vertex_edge_vertex(const DT& dt, const std::size_t& nb_tests)
{
  std::cout << "* test_vertex_edge_vertex *" << std::endl;
  std::vector<Edge> edges;
  for (DT::Finite_edges_iterator eit = dt.finite_edges_begin();
       eit != dt.finite_edges_end() && edges.size() < nb_tests;
       ++eit)
  {
    edges.push_back(*eit);
  }

  for (std::size_t i = 0; i < nb_tests; ++i)
  {
    Vertex_handle v1 = edges[i].first->vertex(edges[i].second);
    Vertex_handle v2 = edges[i].first->vertex(edges[i].third);
    Vector_3 v(v1->point(), v2->point());

    std::cout << "TEST " << i << " (" << v1->point()
                          << " ** " << v2->point() <<")"
                          << std::endl;
    std::cout << "\t(";
    Simplex_traverser st(dt, v1->point() - 2.*v, v2->point() + 3.*v);
    for (; st != st.end(); ++st)
    {
      std::cout << st.simplex_dimension();
      if(dt.is_infinite(st))
        std::cout << "i";
      std::cout << " ";

      if (st.is_vertex() && st.get_vertex() == v1)
      {
        ++st;
        std::cout << st.simplex_dimension() << " ";
        assert(st.is_edge());
        Edge e = st.get_edge();
        Vertex_handle ve1 = e.first->vertex(e.second);
        Vertex_handle ve2 = e.first->vertex(e.third);
        assert((ve1 == v1 && ve2 == v2)
            || (ve1 == v2 && ve2 == v1));

        ++st;
        std::cout << st.simplex_dimension() << " ";
        assert(st.is_vertex());
        assert(st.get_vertex() == v2);
      }
    }
    std::cout << ")" << std::endl;
  }
}

void test_edge_facet_edge(const DT& dt, const std::size_t& nb_tests)
{
  std::cout << "* test_edge_facet_edge *" << std::endl;
  std::vector<Facet> facets;
  for (DT::Finite_facets_iterator fit = dt.finite_facets_begin();
    fit != dt.finite_facets_end() && facets.size() < nb_tests;
    ++fit)
  {
    facets.push_back(*fit);
  }
  for (std::size_t i = 0; i < nb_tests; ++i)
  {
    const int fi = facets[i].second;
    Vertex_handle v1 = facets[i].first->vertex((fi + 1) % 4);
    Vertex_handle v2 = facets[i].first->vertex((fi + 2) % 4);
    Vertex_handle v3 = facets[i].first->vertex((fi + 3) % 4);

    Point_3 p1 = CGAL::midpoint(v1->point(), v2->point());
    Point_3 p2 = CGAL::midpoint(v2->point(), v3->point());
    Vector_3 v(p1, p2);

    std::cout << "TEST " << i << " (" << p1 << " ** " << p2 << ")"
      << std::endl;
    std::cout << "\t(";
    Simplex_traverser st(dt, p1 - 2.*v, p2 + 3.*v);
    for (; st != st.end(); ++st)
    {
      std::cout << st.simplex_dimension();
      if (dt.is_infinite(st))
        std::cout << "i";
      std::cout << " ";

      if (st.is_edge())
      {
        Edge e = st.get_edge();
        Vertex_handle va = e.first->vertex(e.second);
        Vertex_handle vb = e.first->vertex(e.third);
        if ((va == v1 && vb == v2)
          || (va == v2 && vb == v1))
        {
          ++st;
          std::cout << st.simplex_dimension() << " ";
          assert(st.is_facet());

          ++st;
          std::cout << st.simplex_dimension() << " ";
          assert(st.is_edge());
          Edge e2 = st.get_edge();
          Vertex_handle va2 = e2.first->vertex(e2.second);
          Vertex_handle vb2 = e2.first->vertex(e2.third);
          assert(va == va2 || va == vb2 || vb == va2 || vb == vb2);
        }
      }
    }
    std::cout << ")" << std::endl;
  }
}

void test_edge_facet_vertex(const DT& dt, const std::size_t& nb_tests)
{
  std::cout << "* test_edge_facet_vertex *" << std::endl;
  std::vector<Facet> facets;
  for (DT::Finite_facets_iterator fit = dt.finite_facets_begin();
    fit != dt.finite_facets_end() && facets.size() < nb_tests;
    ++fit)
  {
    facets.push_back(*fit);
  }
  for (std::size_t i = 0; i < nb_tests; ++i)
  {
    const int fi = facets[i].second;
    Vertex_handle v1 = facets[i].first->vertex((fi + 1) % 4);
    Vertex_handle v2 = facets[i].first->vertex((fi + 2) % 4);
    Vertex_handle v3 = facets[i].first->vertex((fi + 3) % 4);

    Point_3 p1 = CGAL::midpoint(v1->point(), v2->point());
    Point_3 p2 = v3->point();
    Vector_3 v(p1, p2);

    std::cout << "TEST " << i << " (" << p1 << " ** " << p2 << ")"
      << std::endl;
    std::cout << "\t(";
    Simplex_traverser st(dt, p1 - 2.*v, p2 + 3.*v);
    Simplex_traverser end = st.end();
    for (; st != end; ++st)
    {
      std::cout << st.simplex_dimension();
      if (dt.is_infinite(st))
        std::cout << "i";
      std::cout << " ";

      if (st.is_edge())
      {
        Edge e = st.get_edge();
        Vertex_handle va = e.first->vertex(e.second);
        Vertex_handle vb = e.first->vertex(e.third);
        if ((va == v1 && vb == v2) || (va == v2 && vb == v1))
        {
          ++st;
          std::cout << st.simplex_dimension() << " ";
          assert(st.is_facet());

          ++st;
          std::cout << st.simplex_dimension() << " ";
          assert(st.is_vertex());
          assert(st.get_vertex() == v3);
        }
        ++st;
        std::cout << st.simplex_dimension() << " ";
        if (st == st.end())
          break;
        else if (dt.is_infinite(st)) std::cout << "i ";
        assert(st.is_cell());
      }
    }
    std::cout << ")" << std::endl;
  }
}

void test_vertex_facet_edge(const DT& dt, const std::size_t& nb_tests)
{
  std::cout << "* test_vertex_facet_edge *" << std::endl;
  std::vector<Facet> facets;
  DT::Finite_facets_iterator fit = dt.finite_facets_begin();
  ++fit; ++fit; ++fit; //just avoid using the same faces as for test_edge_facet_vertex
  for (; fit != dt.finite_facets_end() && facets.size() < nb_tests;
       ++fit)
  {
    facets.push_back(*fit);
  }
  for (std::size_t i = 0; i < nb_tests; ++i)
  {
    const int fi = facets[i].second;
    Vertex_handle v1 = facets[i].first->vertex((fi + 1) % 4);
    Vertex_handle v2 = facets[i].first->vertex((fi + 2) % 4);
    Vertex_handle v3 = facets[i].first->vertex((fi + 3) % 4);

    Point_3 p1 = v1->point();
    Point_3 p2 = CGAL::midpoint(v2->point(), v3->point());
    Vector_3 v(p1, p2);

    std::cout << "TEST " << i << " (" << p1 << " ** " << p2 << ")"
      << std::endl;
    std::cout << "\t(";
    Simplex_traverser st(dt, p1 - 2.*v, p2 + 3.*v);
    Simplex_traverser end = st.end();
    for (; st != end; ++st)
    {
      std::cout << st.simplex_dimension();
      if (dt.is_infinite(st))
        std::cout << "i";
      std::cout << " ";

      if (st.is_vertex() && st.get_vertex() == v1)
      {
        ++st;
        std::cout << st.simplex_dimension() << " ";
        assert(st.is_facet());
        assert(st.get_facet() == facets[i]
            || st.get_facet() == dt.mirror_facet(facets[i]));
        ++st;
        std::cout << st.simplex_dimension() << " ";
        assert(st.is_edge());
        Edge e = st.get_edge();
        Vertex_handle va = e.first->vertex(e.second);
        Vertex_handle vb = e.first->vertex(e.third);
        assert((va == v2 && vb == v3) || (va == v3 && vb == v2));
      }
    }
    std::cout << ")" << std::endl;
  }
}


int main(int argc, char* argv[])
{
  const char* fname = (argc>1) ? argv[1] : "data/blobby.xyz";
  int nb_seg = (argc > 2) ? atoi(argv[2]) : 3;

  // Reads a .xyz point set file in points.
  // As the point is the second element of the tuple (that is with index 1)
  // we use a property map that accesses the 1st element of the tuple.

  std::vector<Point_3> points;
  std::ifstream stream(fname);
  if (!stream ||
    !CGAL::read_xyz_points(stream, std::back_inserter(points)))
  {
    std::cerr << "Error: cannot read file " << fname << std::endl;
    return EXIT_FAILURE;
  }

  //bbox
  //min (-0.481293,-0.220929,-0.194076), max (0.311532,0.225525,0.198025)

  // Construct the Delaunay triangulation.
  DT dt(points.begin(), points.end());
  assert(dt.is_valid());

  CGAL::Random rng;
  for (int i = 0; i < nb_seg; ++i)
  {
    // Construct a traverser.
    Point_3 p1(rng.get_double(-0.48, 0.31),
               rng.get_double(-0.22, 0.22),
               rng.get_double(-0.19, 0.19));
    Point_3 p2(rng.get_double(-0.48, 0.31),
               rng.get_double(-0.22, 0.22),
               rng.get_double(-0.19, 0.19));

    std::cout << "Traverser " << (i + 1)
      << "\n\t(" << p1
      << ")\n\t(" << p2 << ")" << std::endl;
    Simplex_traverser st(dt, p1, p2);

    // Count the number of finite cells traversed.
    unsigned int inf = 0, fin = 0;
    unsigned int nb_facets = 0, nb_edges = 0, nb_vertex = 0;
    unsigned int nb_collinear = 0;
    for (; st != st.end(); ++st)
    {
      if (dt.is_infinite(st))
        ++inf;
      else
      {
        ++fin;
        if (st.is_facet())       ++nb_facets;
        else if (st.is_edge())   ++nb_edges;
        else if (st.is_vertex()) ++nb_vertex;

        if (st.is_collinear())   ++nb_collinear;
      }
    }

    std::cout << "While traversing from " << st.source()
      << " to " << st.target() << std::endl;
    std::cout << "\tinfinite cells : " << inf << std::endl;
    std::cout << "\tfinite cells   : " << fin << std::endl;
    std::cout << "\tfacets   : " << nb_facets << std::endl;
    std::cout << "\tedges    : " << nb_edges << std::endl;
    std::cout << "\tvertices : " << nb_vertex << std::endl;
    std::cout << std::endl << std::endl;
  }

  //check degenerate cases
  // - along an edge
  test_vertex_edge_vertex(dt, 2);

  // - along a facet via edge/facet/edge
  test_edge_facet_edge(dt, 3);

  // - along a facet via edge/facet/vertex
  test_edge_facet_vertex(dt, 3);

  // - along a facet via vertex/facet/edge
  test_vertex_facet_edge(dt, 3);

  // - along 2 successive facets (vertex/facet/edge/facet/edge)
  // - along 2 successive edges (vertex/edge/vertex/edge/vertex)
  // - along a facet and an edge successively

  return 0;
}