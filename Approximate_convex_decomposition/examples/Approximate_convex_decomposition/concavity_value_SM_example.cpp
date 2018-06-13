#include <CGAL/approx_decomposition.h>
#include <CGAL/Surface_mesh.h>

#include <iostream>
#include <fstream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Surface_mesh;

int main()
{
    // read mesh
    Surface_mesh mesh;
    
    std::ifstream input("data/elephant.off");
    
    if (!input || !(input >> mesh))
    {
        std::cout << "Failed to read mesh" << std::endl;
        return EXIT_FAILURE;
    }

    if (CGAL::is_empty(mesh) || !CGAL::is_triangle_mesh(mesh))
    {
        std::cout << "Input mesh is invalid" << std::endl;
        return EXIT_FAILURE;
    }

    // compute concavity value
    double concavity = CGAL::concavity_value(mesh);

    // write result
    std::cout << "Concavity value: " << concavity << std::endl;

    return EXIT_SUCCESS;
}
