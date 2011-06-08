//! \file examples/Arrangement_2/unbounded_rational_functions.cpp
// Constructing an arrangement of unbounded portions of rational functions.
#include <CGAL/basic.h>

#include <CGAL/CORE_BigInt.h>                    //NT
#include <CGAL/Algebraic_kernel_d_1.h>           //Algebraic Kernel
#include <CGAL/Arr_rational_function_traits_2.h> //Traits
#include <CGAL/Arrangement_2.h>                  //Arrangement

typedef CORE::BigInt	                           Number_type;
typedef CGAL::Algebraic_kernel_d_1<Number_type>	   AK1;
typedef CGAL::Arr_rational_function_traits_2<AK1>  Traits_2;

typedef Traits_2::Polynomial_1                     Polynomial_1;
typedef Traits_2::Algebraic_real_1                 Alg_real_1;

typedef CGAL::Arrangement_2<Traits_2>              Arrangement_2;

int main ()
{
  CGAL::set_pretty_mode(std::cout);             // for nice printouts.

  // Traits class object 
  AK1 ak1; 
  Traits_2 traits(&ak1);
    
  // constructor for rational functions 
  Traits_2::Construct_curve_2 construct
    = traits.construct_curve_2_object(); 
  
  // a polynomial representing x .-)
  Polynomial_1 x = CGAL::shift(Polynomial_1(1),1);
  
  // container storing all arcs 
  std::vector<Traits_2::Curve_2>  arcs;

  
  // Create the rational functions (y = 1 / x), and (y = -1 / x).
  Polynomial_1 P1(1);
  Polynomial_1 minusP1(-P1);
  Polynomial_1 Q1 = x;
  arcs.push_back(construct( P1.begin(), P1.end(), Q1.begin(),Q1.end()));
  arcs.push_back(construct(minusP1.begin(),minusP1.end(), Q1.begin(),Q1.end()));

  // Create a bounded segments of the parabolas (y = -4*x^2 + 3) and
  // (y = 4*x^2 - 3), defined over [-sqrt(3)/2, sqrt(3)/2].
  Polynomial_1 P2 = -4*x*x+3; 
  Polynomial_1 minusP2 = -P2; 
  std::vector<std::pair<Alg_real_1,int> > roots;
  traits.algebraic_kernel_d_1()->solve_1_object()(P2,std::back_inserter(roots));// [-sqrt(3)/2, sqrt(3)/2]
  arcs.push_back(construct(P2.begin(), P2.end(), roots[0].first, roots[1].first));
  arcs.push_back(construct(minusP2.begin(),minusP2.end(), roots[0].first, roots[1].first));

  // Create the rational function (y = 1 / 2*x) for x > 0, and the
  // rational function (y = -1 / 2*x) for x < 0.
  Polynomial_1 P3(1);
  Polynomial_1 minusP3(-P3);
  Polynomial_1 Q3 = 2*x;
  arcs.push_back(construct( P3.begin(), P3.end(), Q3.begin(), Q3.end(), Alg_real_1(0), true ));
  arcs.push_back(construct(minusP3.begin(),minusP3.end(), Q3.begin(), Q3.end(), Alg_real_1(0), false));

  // Construct the arrangement of the six arcs.
  //Arrangement_2 arr(&traits);
  Arrangement_2 arr;
  insert (arr, arcs.begin(), arcs.end());

  // Print the arrangement size.
  std::cout << "The arrangement size:" << std::endl
    << "   V = " << arr.number_of_vertices()
    << " (plus " << arr.number_of_vertices_at_infinity()
    << " at infinity)"
    << ",  E = " << arr.number_of_edges() 
    << ",  F = " << arr.number_of_faces() 
    << " (" << arr.number_of_unbounded_faces() << " unbounded)"
    << std::endl << std::endl;

  return 0;
}
