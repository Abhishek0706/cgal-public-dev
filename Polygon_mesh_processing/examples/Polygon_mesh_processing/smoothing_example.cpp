#include <iostream>
#include <fstream>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/smoothing.h>


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;



int main(int argc, char* argv[]){


    const char* filename = "data/polygon3D.off";
    std::ifstream input(filename);

    Mesh mesh;
    if (!input || !(input >> mesh) || mesh.is_empty()) {
        std::cerr << "Not a valid .off file." << std::endl;
        return 1;
    }



    CGAL::Polygon_mesh_processing::compatible_remeshing(
                                    mesh,
                                    faces(mesh),
                                    edges(mesh),
                                    CGAL::Polygon_mesh_processing::parameters::number_of_iterations(3));



    std::ofstream output("data/polygon3D_smoothed.off");
    output << mesh;
    output.close();



    return 0;
}