
namespace CGAL {

/*!
\ingroup PkgGeneralizedMapsClasses

The class `Generalized_map_min_items` is a model of the `GeneralizedMapItems` concept. It defines the type of darts which is a `GDart<d,GMap>`. The `Generalized_map_min_items` has a template argument for the dimension of the generalized map. In this class, no attribute is enabled.

\cgalModels `GeneralizedMapItems`

\cgalHeading{Example}

The following example shows the implementation of the `Generalized_map_min_items` class.

\code{.cpp}
template <unsigned int d>
struct Generalized_map_min_items
{
  template <class GMap>
  struct Dart_wrapper
  {
    typedef CGAL::GDart<d, GMap> Dart;
    typedef CGAL::cpp11::tuple<> Attributes;
  };
};
\endcode

\sa `Generalized_map<d,Items,Alloc>`
\sa `GDart<d,GMap>`

*/
template< typename d >
class Generalized_map_min_items {
public:

/// @}

}; /* end Generalized_map_min_items */
} /* end namespace CGAL */
