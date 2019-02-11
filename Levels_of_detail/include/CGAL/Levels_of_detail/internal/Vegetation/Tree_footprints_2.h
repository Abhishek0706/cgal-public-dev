#ifndef CGAL_LEVELS_OF_DETAIL_TREE_FOOTPRINTS_2_H
#define CGAL_LEVELS_OF_DETAIL_TREE_FOOTPRINTS_2_H

// STL includes.
#include <cmath>
#include <vector>

namespace CGAL {
namespace Levels_of_detail {
namespace internal {

  template<typename GeomTraits>
  class Tree_footprints_2 {

  public:
    using Traits = GeomTraits;
    using FT = typename Traits::FT;
    using Point_2 = typename Traits::Point_2;
    using Vector_2 = typename Traits::Vector_2;
    using Triangle_2 = typename Traits::Triangle_2;

    void create_footprint_triangles(
      const Point_2& center,
      const FT radius,
      const std::size_t min_faces_per_tree,
      std::vector<Triangle_2>& triangles) const {

      const std::size_t n = min_faces_per_tree;
      
      triangles.clear();
      triangles.resize(n);

      std::vector<Point_2> points(n);
      for (std::size_t i = 0; i < n; ++i) {

        const FT angle = 
        FT(2) * static_cast<FT>(CGAL_PI) * 
        (static_cast<FT>(i) / static_cast<FT>(n));

        points[i] = center + Vector_2(
          radius * static_cast<FT>(std::cos(CGAL::to_double(angle))),
          radius * static_cast<FT>(std::sin(CGAL::to_double(angle))));
      }

      for (std::size_t i = 0; i < n; ++i) {
        const std::size_t ip = (i + 1) % n;

        const Point_2& p1 = points[i];
        const Point_2& p2 = points[ip];

        triangles[i] = Triangle_2(center, p1, p2);
      }
    }

  }; // Tree_footprints_2

} // internal
} // Levels_of_detail
} // CGAL

#endif // CGAL_LEVELS_OF_DETAIL_TREE_FOOTPRINTS_2_H
