#ifndef CGAL_CONCAVITY_H
#define CGAL_CONCAVITY_H

#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <boost/foreach.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace CGAL
{
namespace internal
{

    template <class TriangleMesh, class GeomTraits>
    class Concavity
    {
        typedef typename GeomTraits::Point_3 Point_3;
        typedef typename GeomTraits::Vector_3 Vector_3;
        typedef typename GeomTraits::Ray_3 Ray_3;
        
        typedef typename boost::graph_traits<TriangleMesh>::vertex_descriptor vertex_descriptor;
        typedef typename boost::graph_traits<TriangleMesh>::face_descriptor face_descriptor;

        typedef typename boost::graph_traits<TriangleMesh>::vertex_iterator vertex_iterator;
        
        typedef std::map<vertex_descriptor, Vector_3> Normals_map;

        typedef CGAL::Face_filtered_graph<TriangleMesh> Filtered_graph;
        typedef CGAL::AABB_face_graph_triangle_primitive<TriangleMesh> AABB_primitive;
        typedef CGAL::AABB_tree<CGAL::AABB_traits<GeomTraits, AABB_primitive>> AABB_tree;

        typedef boost::optional<typename AABB_tree::template Intersection_and_primitive_id<Ray_3>::Type> Ray_intersection;
    
    public:
        Concavity(const TriangleMesh& mesh, const GeomTraits& traits)
        : m_mesh(mesh)
        , m_traits(traits)
        {}

        template <class FacetPropertyMap>
        double compute(FacetPropertyMap facet_ids, std::size_t cluster_id)
        {
            Filtered_graph filtered_mesh(m_mesh, cluster_id, facet_ids);

            TriangleMesh cluster;
            CGAL::copy_face_graph(filtered_mesh, cluster);

#ifdef CGAL_APPROX_DECOMPOSITION_VERBOSE
//            {
//                std::ofstream os("cluster_" + std::to_string(cluster_id) + ".off");
//                os << cluster;
//            }
//            {
//                TriangleMesh conv_hull;
//                std::vector<Point_3> pts;

//                if (CGAL::num_vertices(cluster) > 3)
//                {
//                    BOOST_FOREACH(vertex_descriptor vert, CGAL::vertices(cluster))
//                    {
//                        pts.push_back(cluster.point(vert));
//                    }

//                    CGAL::convex_hull_3(pts.begin(), pts.end(), conv_hull);
//                }
//                else
//                {
//                    conv_hull = cluster;
//                }
//                std::ofstream os("ch_cluster_" + std::to_string(cluster_id) + ".off");
//                os << conv_hull;
//            }
#endif

            Concavity concavity(cluster, m_traits);
            return concavity.compute();
        }

        double compute()
        {
            CGAL_assertion(!CGAL::is_empty(m_mesh));

            TriangleMesh conv_hull;
            std::vector<Point_3> pts;

            if (CGAL::num_vertices(m_mesh) <= 3) return 0;

            BOOST_FOREACH(vertex_descriptor vert, vertices(m_mesh))
            {
                pts.push_back(get(CGAL::vertex_point, m_mesh)[vert]);
            }

            CGAL::convex_hull_3(pts.begin(), pts.end(), conv_hull); 
            
            return compute(vertices(m_mesh), conv_hull);
        }

        double compute(const std::vector<face_descriptor>& faces, const TriangleMesh& conv_hull)
        {
            return 0;
        }

        double compute(const std::pair<vertex_iterator, vertex_iterator>& verts, const TriangleMesh& conv_hull)
        {
            compute_normals();

            AABB_tree tree(faces(conv_hull).begin(), faces(conv_hull).end(), conv_hull);

            double result = 0;

            BOOST_FOREACH(vertex_descriptor vert, verts)
            {
                Point_3 origin = get(CGAL::vertex_point, m_mesh)[vert];
                Ray_3 ray(origin, m_normals_map[vert]);
                
                Ray_intersection intersection = tree.first_intersection(ray);
                if (intersection)
                {
                    const Point_3* p =  boost::get<Point_3>(&(intersection->first));
                    if (p)
                    {
                        result = std::max(result, CGAL::squared_distance(origin, *p));
                    }
                }
            }

            return CGAL::sqrt(result);
        }

    private:
        const TriangleMesh& m_mesh;
        const GeomTraits& m_traits;

        Normals_map m_normals_map;
        bool m_normals_computed = false;

        void compute_normals()
        {
            if (m_normals_computed) return;

            CGAL::Polygon_mesh_processing::compute_vertex_normals(m_mesh, boost::associative_property_map<Normals_map>(m_normals_map));
            m_normals_computed = true;
        }
    };

}
}

#endif // CGAL_CONCAVITY_H