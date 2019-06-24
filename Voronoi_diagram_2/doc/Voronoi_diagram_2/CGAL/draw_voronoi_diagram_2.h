namespace CGAL {
  
/*!
\ingroup PkgDrawVoronoiDiagram2

Open a new window and draw `av2`, the `Voronoi_diagram_2` constructed from a Delaunay Graph which is a model of `DelaunayGraph_2` concept.
The class `Voronoi_diagram_2` provides an adaptor to view a triangulated Delaunay graph as their dual subdivision, the 
Voronoi diagram. The function is blocking, that is the program continues as soon as the user closes the window. 
 This function requires CGAL_Qt5, and is only available if the flag CGAL_USE_BASIC_VIEWER is defined at compile time.
\tparam V2 a model of the `Voronoi2` concept.
\param av2 the voronoi diagram to draw.

*/
template<class V2>
void draw(const V2& av2);

} /* namespace CGAL */