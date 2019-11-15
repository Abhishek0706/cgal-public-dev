#include <vector>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Barycentric_coordinates_2/analytic_coordinates_2.h>

// Typedefs.
using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;

using FT      = Kernel::FT;
using Point_2 = Kernel::Point_2;

int main() {

  const FT y = FT(2) / FT(5);

  // Construct a segment.
  const Point_2 source(FT(0), y);
  const Point_2 target(FT(2), y);

  // Instantiate three interior and two exterior query points.
  const std::vector<Point_2> query_points = { 
    Point_2(FT(2) / FT(5), y), // interior query points
    Point_2(FT(5) / FT(5), y),
    Point_2(FT(8) / FT(5), y),
    
    Point_2(-FT(1) / FT(5), y), // exterior query points
    Point_2(FT(11) / FT(5), y) };

  // Compute segment coordinates.
  std::vector<FT> coordinates;
  coordinates.reserve(query_points.size() * 2);

  for(const auto& query_point : query_points)
    CGAL::Barycentric_coordinates::segment_coordinates_2(
      source, target, query_point, std::back_inserter(coordinates));

  // Output segment coordinates.
  std::cout << std::endl << "segment coordinates: " << std::endl << std::endl;
  for (std::size_t i = 0; i < coordinates.size(); i += 2)
    std::cout << 
    coordinates[i + 0] << ", " << coordinates[i + 1] 
    << std::endl;
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
