#ifndef CGAL_SHAPE_REGULARIZATION_INTERNAL_SEGMENT_DATA_2
#define CGAL_SHAPE_REGULARIZATION_INTERNAL_SEGMENT_DATA_2

// #include <CGAL/license/Shape_regularization.h>

#include <CGAL/Shape_regularization/internal/utils.h>

namespace CGAL {
namespace Regularization {
namespace internal {

  template<
    typename GeomTraits>
  struct Segment_data_2 {

  public:
    using Traits = GeomTraits;
    using Segment = typename GeomTraits::Segment_2;
    using Point = typename GeomTraits::Point_2;
    using Vector  = typename GeomTraits::Vector_2;
    using FT = typename GeomTraits::FT;

    const std::size_t  m_index;
    const Segment& m_segment;
    Vector  m_direction;
    FT      m_orientation;
    Point   m_barycentre;
    FT      m_length;
    Point   m_reference_coordinates;

    Segment_data_2(
      const Segment& segment,
      const std::size_t index):
    m_segment(segment),
    m_index(index) {
      m_direction = compute_direction(m_segment);
      m_orientation = compute_orientation(m_direction);
      m_barycentre = compute_middle_point(m_segment.source(), m_segment.target());
      m_length = static_cast<FT>(CGAL::sqrt(CGAL::to_double(m_segment.squared_length())));

    }

    void set_reference_coordinates(const Point & reference_coordinates) {
      m_reference_coordinates = reference_coordinates;
    }

  private:
    // FT      m_difference;

  };

} // namespace internal
} // namespace Regularization
} // namespace CGAL

#endif // CGAL_SHAPE_REGULARIZATION_INTERNAL_SEGMENT_DATA_2