// TODO: Add licence
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL:$
// $Id: $
// 
//
// Author(s)     : Sebastian Limbach <slimbach@mpi-inf.mpg.de>
//                 Michael Hemmer    <hemmer@mpi-inf.mpg.de> 
//
// ============================================================================

// DONE: test solve_1_object(), etc.
// TODO: Use proper construction of Polynomials 

#include <CGAL/basic.h>
#include <cassert>
#include <CGAL/Algebraic_kernel_1.h>
#include <CGAL/Test/_test_real_embeddable.h>
#include <CGAL/Test/_test_algebraic_structure.h>
#include <CGAL/Test/_test_coercion_traits.h>
#include <CGAL/Test/_test_polynomial_traits_d.h>


// Test for the Algebraic_kernel syntax
#ifndef CGAL_TEST_ALGEBRAIC_KERNEL_1_H
#define CGAL_TEST_ALGEBRAIC_KERNEL_1_H

CGAL_BEGIN_NAMESPACE


// Test for an exact AlgebraicKernel1
template <class AlgebraicKernel_d_1>
void test_algebraic_kernel_1(const AlgebraicKernel_d_1& ak_1){
  typedef AlgebraicKernel_d_1 Algebraic_kernel_d_1;

  typedef typename AlgebraicKernel_d_1::Coefficient Coefficient;
  { 
    // check Coefficient 
    typedef Algebraic_structure_traits<Coefficient> AST;
    typedef typename AST::Algebraic_category Algebraic_category; 
    test_algebraic_structure< Coefficient,Algebraic_category,Tag_true>();
    test_real_embeddable<Coefficient>();
  }
  typedef typename AlgebraicKernel_d_1::Polynomial_1 Polynomial_1; 
  {
    // check Polynomial_1
    typedef Polynomial_traits_d<Polynomial_1> PT;
    test_polynomial_traits_d(PT());

    // test not possible due to bug in test_algebraic_structure
    // div(3,2)=3/2 != 0 in case of Polynomial<Rational> 
    //  typedef Algebraic_structure_traits<Polynomial_1> AST;
    //  typedef typename AST::Algebraic_category Algebraic_category; 
    //  test_algebraic_structure< Polynomial_1,Algebraic_category,Tag_true>();
  }
  typedef typename AlgebraicKernel_d_1::Algebraic_real_1 Algebraic_real_1;
  {
    // check Algebraic_real_1
    test_real_embeddable<Algebraic_real_1>();
  }
  typedef typename AlgebraicKernel_d_1::Bound Bound;
  { 
    typedef Algebraic_structure_traits<Bound> AST;
    typedef typename AST::Algebraic_category Algebraic_category; 
    test_algebraic_structure< Bound,Algebraic_category,Tag_true>();
    test_real_embeddable<Bound>();
  }
  
  test_explicit_interoperable_from_to<int, Coefficient>();
  test_explicit_interoperable_from_to<int, Bound>();

  test_explicit_interoperable_from_to<int        , Algebraic_real_1>();
  test_explicit_interoperable_from_to<Bound   , Algebraic_real_1>();
  test_explicit_interoperable_from_to<Coefficient, Algebraic_real_1>();
}




namespace CGALi {

template< 
class AK_, 
class AlgebraicReal1, 
class Isolator_, 
class Coefficient_, 
class Polynomial1, 
class Bound_  >
void old_test_algebraic_kernel_1() {
    typedef AK_            AK;
    typedef AlgebraicReal1 Algebraic_real_1;
    typedef Isolator_      Isolator;
    typedef Coefficient_   Coefficient;
    typedef Polynomial1    Polynomial_1;
    typedef Bound_      Bound;
        
    BOOST_STATIC_ASSERT( (::boost::is_same< 
            Algebraic_real_1, typename AK::Algebraic_real_1 >::value) );

    BOOST_STATIC_ASSERT((::boost::is_same<
            Isolator,
            typename AK::Isolator >::value) );
            
    BOOST_STATIC_ASSERT((::boost::is_same< 
            Coefficient, 
            typename AK::Coefficient >::value));
            
    BOOST_STATIC_ASSERT((::boost::is_same<
            Polynomial_1,
            typename AK::Polynomial_1 >::value));
    
    // Test of functors
    // Test AK::Solve_1...
    
    typename AK::Solve_1 solve_1 = AK().solve_1_object();
    Polynomial_1 poly1( -4,0,1 );
    Polynomial_1 poly2( 0, 0, 1 );
    std::vector< Algebraic_real_1 > roots_vec;
    std::vector< int > mults_vec;
    
    solve_1( poly1, std::back_inserter( roots_vec ) );        
    assert( roots_vec.size() == 2 );
    assert( CGAL::abs( roots_vec[0] ) == CGAL::abs( Algebraic_real_1(2) ) );
    roots_vec.clear();
    
    solve_1( poly1, std::back_inserter( roots_vec ), std::back_inserter( mults_vec ) );
    assert( roots_vec.size() == 2 );
    assert( mults_vec.size() == 2 );
    assert( CGAL::abs( roots_vec[0] ) == CGAL::abs( Algebraic_real_1(2) ) );
    assert( CGAL::abs( roots_vec[1] ) == CGAL::abs( Algebraic_real_1(2) ) );
    assert( mults_vec[0] == 1 );
    assert( mults_vec[1] == 1 );
    roots_vec.clear();
    mults_vec.clear();

    solve_1( poly2, std::back_inserter( roots_vec ), std::back_inserter( mults_vec ) );
    assert( roots_vec.size() == 1 );
    assert( mults_vec.size() == 1 );
    assert( CGAL::abs( roots_vec[0] ) == CGAL::abs( Algebraic_real_1(0) ) );
    assert( mults_vec[0] == 2 );        
    roots_vec.clear();
    mults_vec.clear();
    
    // Test AK::Sign_at_1
    typename AK::Sign_at_1 sign_at_1;
    typename AK::Polynomial_1 poly4( -2,0,1 );
    solve_1( poly4, std::back_inserter( roots_vec ) );
    typename AK::Polynomial_1 poly3( 0,0,0,1 ); 
    assert( sign_at_1( poly3, roots_vec[0] ) == CGAL::sign( roots_vec[0] ) );
    assert( sign_at_1( poly3, roots_vec[1] ) == CGAL::sign( roots_vec[1] ) );
    assert( sign_at_1( poly3, Algebraic_real_1(0) ) == CGAL::ZERO );  
    roots_vec.clear();
    
    solve_1( poly1, std::back_inserter( roots_vec ) );
    assert( sign_at_1( poly3, roots_vec[0] ) == CGAL::sign( roots_vec[0] ) );
    assert( sign_at_1( poly3, roots_vec[1] ) == CGAL::sign( roots_vec[1] ) );
    assert( sign_at_1( poly3, Algebraic_real_1(0) ) == CGAL::ZERO );  
    roots_vec.clear();
    
    typename AK::Polynomial_1 poly5( 0,0,-1,0,1 );
    typename AK::Algebraic_real_1 algreal1( poly1, Bound(-3), Bound(1) );
    typename AK::Algebraic_real_1 algreal2( poly1, Bound(-1), Bound(3) );
    assert( sign_at_1( poly5, algreal2 ) == CGAL::POSITIVE );
    
    
    // Just syntax tests... (TODO)
    // Test AK::Is_square_free_1...
    typename AK::Is_square_free_1 is_square_free_1 
      = AK().is_square_free_1_object();
    is_square_free_1( poly1 );
    
    // Test AK::Is_coprime_1...
    typename AK::Is_coprime_1 is_coprime_1
      = AK().is_coprime_1_object();
    is_coprime_1( poly1, poly2 );
        
    // Test AK::Make_square_free_1...
    typename AK::Make_square_free_1 make_square_free_1
      = AK().make_square_free_1_object();
    make_square_free_1( poly1 );
    
    // Test AK::Make_coprime_1...
    typename AK::Make_coprime_1 make_coprime_1 
      = AK().make_coprime_1_object();
    Polynomial_1 g, q1, q2;
    make_coprime_1( poly1, poly2, g, q1, q2 );
    
    // Test AK::Square_free_factorize_1...
    typename AK::Square_free_factorize_1 square_free_factorize_1 
      = AK().square_free_factorize_1_object();
    
    std::vector<std::pair<Polynomial_1,int> > fac_mult_pairs;
    square_free_factorize_1( poly1, std::back_inserter(fac_mult_pairs) );
        
    ////////////////////////////////////////////////////////////////////////////
    
    // (Not only) syntax tests for Algebraic_real_traits
            
    // Create test polynomial
    Polynomial_1 p2( -2,0,1 );
    std::vector< Algebraic_real_1 > roots_vec2;
    
    solve_1( p2, std::back_inserter( roots_vec2 ) );
    
    // Test AK::Bound_between...
    typename AK::Bound_between_1 boundary_between 
      = AK().boundary_between_1_object();
    assert( typename AK::Bound( -2 ) < 
        boundary_between( roots_vec2[0], roots_vec2[1] ) );
    assert( typename AK::Bound(  2 ) > 
        boundary_between( roots_vec2[0], roots_vec2[1] ) );
    
    // Test AK::Lower_boundary
    typename AK::Lower_boundary_1 lower_boundary 
      = AK().lower_boundary_1_object();
    assert( lower_boundary( roots_vec2[0] ) < typename AK::Bound(-1) );
    assert( lower_boundary( roots_vec2[1] ) < typename AK::Bound( 2) );

    // Test AK::Upper_boundary
    typename AK::Upper_boundary_1 upper_boundary
      = AK().upper_boundary_1_object();
    assert( upper_boundary( roots_vec2[0] ) > typename AK::Bound(-1) );
    assert( upper_boundary( roots_vec2[1] ) > typename AK::Bound( 1) );
    
    // Test AK::Refine
    typename AK::Refine_1 refine
      = AK().refine_1_object();
    Algebraic_real_1 ar = roots_vec2[1];
    typename AK::Bound old_lower_boundary = ar.low();
    typename AK::Bound old_upper_boundary = ar.high(); 

    refine( ar );
    
    assert( old_lower_boundary <= lower_boundary( ar ) );
    assert( old_upper_boundary >= upper_boundary( ar ) );
    typename AK::Bound interval_size_old 
      = CGAL::abs( old_upper_boundary - old_lower_boundary );
    typename AK::Bound interval_size_new 
      = CGAL::abs( upper_boundary( ar ) - lower_boundary( ar ) );
    assert( interval_size_new * typename AK::Bound(2) <= interval_size_old );
    
    refine( ar, 100 );
    assert( CGAL::abs( upper_boundary( ar ) - lower_boundary( ar ) ) < 
        (typename AK::Bound(1) / CGAL::ipower(typename AK::Bound(2), 99 )) );

}

} //namespace CGALi




CGAL_END_NAMESPACE

#endif //CGAL_TEST_ALGEBRAIC_KERNEL_1_H
