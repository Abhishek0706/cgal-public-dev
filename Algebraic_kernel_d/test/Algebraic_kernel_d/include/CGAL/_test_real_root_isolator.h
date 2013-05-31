// Copyright (c) 2006-2009 Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://hemmer@scm.gforge.inria.fr/svn/cgal/trunk/Polynomial/include/CGAL/Polynomial.h $
// $Id: Polynomial.h 47254 2008-12-06 21:18:27Z afabri $
// 
//
// Author(s)     :  Michael Hemmer <hemmer@mpi-inf.mpg.de> 
//
// ============================================================================

// TODO: The comments are all original EXACUS comments and aren't adapted. So
//         they may be wrong now.

#include <CGAL/basic.h>
#include <cassert>
#include <vector>

#include <CGAL/config.h>
#include <CGAL/Arithmetic_kernel.h>
#include <CGAL/ipower.h>

// to get some macros
#include <CGAL/Algebraic_kernel_d/Generic_isolator.h>

#ifndef CGAL_TEST_REAL_ROOT_ISOLATOR_H
#define CGAL_TEST_REAL_ROOT_ISOLATOR_H 1

#ifndef CGAL_TEST_REAL_ROOT_INTERVAL_ISOLATOR
// this tests a constructor that is not part of the concept, but highly recommended
#define CGAL_TEST_REAL_ROOT_INTERVAL_ISOLATOR 0
#endif

namespace CGAL {
  
namespace internal {
	
template <class RealRootIsolator>
int check_intervals_real_root_isolator( 
        const typename RealRootIsolator::Polynomial& p) {

    typedef RealRootIsolator Isolator;
    typedef typename Isolator::Bound Bound;
   
    typedef typename CGAL::Get_arithmetic_kernel< Bound >::Arithmetic_kernel AK;
    typedef typename AK::Rational Rational;
 
    Isolator isolator(p);
    int n = isolator.number_of_real_roots();
    assert(isolator.polynomial() == p); 
    for(int i = 0; i < n; i++) {
       Rational left  = isolator.left_bound(i);
       Rational right = isolator.right_bound(i);

       if (!isolator.is_exact_root(i)) {
           assert(left < right);
	       //std::cout << " left = " << left << std::endl;
	       //std::cout << " right = " << right << std::endl;
	       //std::cout << " p = " << p << std::endl;
           assert(p.sign_at(left) * p.sign_at(right) == CGAL::NEGATIVE);
       }else{
           assert(left == right);
           assert(p.sign_at(left) == CGAL::ZERO);
       }
       // TODO 2012 add tests for all functions in _test_real_root_isolator.h
       // bool is_certainly_simple_root(size_t /* i */) const;
       // bool is_certainly_multiple_root(size_t /* i */) const;
       // int upper_bound_for_multiplicity(size_t /* i */) const;
       // int multiplicity_of_root(size_t /* i */) const;
       // isolator.refine_interval(i);

    }
    return n;
}


#if CGAL_TEST_REAL_ROOT_INTERVAL_ISOLATOR
// Not part of the concept 
 
template <class RealRootIsolator, class Polynomial, class Bound>
int check_intervals_real_root_isolator(const Polynomial& p, 
                                       const Bound& a,
                                       const Bound& b) {
  
  typedef RealRootIsolator Isolator;
  Isolator isolator(p,a,b);
  int n = isolator.number_of_real_roots();
  assert(isolator.polynomial() == p); 
  for (size_t i = 0; static_cast<int>(i) < n; i++) {
    Bound left = isolator.left_bound(i);
    Bound right = isolator.right_bound(i);
    
    assert( left < right || isolator.is_exact_root(i));
    if (!isolator.is_exact_root(i)) {
      assert(left < right);
      //std::cout << " left = " << left << std::endl;
      //std::cout << " right = " << right << std::endl;
      //std::cout << " p = " << p << std::endl;
      assert(p.sign_at(left) * p.sign_at(right) == CGAL::NEGATIVE);
    } else {
      assert(left == right);
      assert(p.sign_at(left) == CGAL::ZERO);
    }
  }
  return n;
};
#endif 

template <class RealRootIsolator>
void test_real_root_isolator() {
    typedef RealRootIsolator Isolator;
    typedef typename Isolator::Polynomial    Polynomial;
    CGAL_USE_TYPE(typename Isolator::Bound);
    typedef typename Polynomial::NT          NT;
 
    // just some Polynomials (not all are used)
    Polynomial P_00(NT(0));                   // zero polynomial
    Polynomial P_01(NT(1));                   // constant polynomial
    Polynomial P_1(NT(-1),NT(1));       //(x-1)
    Polynomial P_2(NT(-2),NT(1));       //(x-2)
    Polynomial P_3(NT(-3),NT(1));       //(x-3)
    Polynomial P_4(NT(-4),NT(1));       //(x-4)
    Polynomial P_12=P_1*P_2;    //(x-1)(x-2)
    Polynomial P_123=P_1*P_2*P_3;    //(x-1)(x-2)(x-3)
    Polynomial P_s2(NT(-2),NT(0),NT(1)); //(x^2-2)
    Polynomial P_s5(-NT(5),NT(0),NT(1)); //(x^2-5)
    Polynomial P_s10(-NT(10),NT(0),NT(1)); //(x^2-10)
    Polynomial P_s30(-NT(30),NT(0),NT(1)); //(x^2-30)
    Polynomial P_s2510= P_s2*P_s5*P_s10;
    Polynomial P_s530= P_s5*P_s30;

    
    
    // special cases: 
    /*
    {
        // default constructor: 
        // as from zero Polynomial
        Isolator isolator;
        assert(isolator.number_of_real_roots() == -1);
        assert(isolator.polynomial() == Polynomial(0)); 
    }{
        // from zero polynomial
        Isolator isolator(P_00);
        assert(isolator.number_of_real_roots() == -1);
        assert(isolator.polynomial() == P_00); 
    }
    */
    {
        // from constant polynomial = 1 
        Polynomial poly(P_01);
        Isolator isolator(poly);
        assert(isolator.number_of_real_roots() == 0);
        assert(isolator.polynomial() == P_01); 
    }
#if !CGAL_ALGEBRAIC_KERNEL_D_ISOLATORS_DISABLE_ASSIGNABLE 
    {
        // copy constructor
        Isolator isolator_1(P_123);
        Isolator isolator_2(isolator_1);
        assert(isolator_1.number_of_real_roots() == 
                isolator_2.number_of_real_roots());
        assert(isolator_1.polynomial() == isolator_2.polynomial()); 
    }{
        // assign
        Isolator isolator_1(P_123);
        Isolator isolator_2 = isolator_1;
        assert(isolator_1.number_of_real_roots() == 
                isolator_2.number_of_real_roots());
        assert(isolator_1.polynomial() == isolator_2.polynomial()); 
    }
#endif
    {  
      assert( 3 == internal::check_intervals_real_root_isolator<Isolator>(P_123));       
    }{  
      assert( 1 == internal::check_intervals_real_root_isolator<Isolator>(P_1));
    }{  
      assert( 3 == internal::check_intervals_real_root_isolator<Isolator>(P_123));    
    }{  
      assert(6 == internal::check_intervals_real_root_isolator<Isolator>(P_s2510));       
    }{  // (x^2-2)*(x^2-3)
      std::vector<NT> VP(5);
      VP[0] = NT(6);
      VP[1] = NT(0);
      VP[2] = NT(-5);
      VP[3] = NT(0);
      VP[4] = NT(1);  
      Polynomial P(VP.begin(), VP.end()); 
      assert(4 == internal::check_intervals_real_root_isolator<Isolator>(P));
    }
    {  // (x^2-2)*(x^2+2)*(x-1)
      std::vector<NT> VP(6);
      VP[0] = NT(4);
      VP[1] = NT(-4);
      VP[2] = NT(0);
      VP[3] = NT(0);
      VP[4] = NT(-1);  
      VP[5] = NT(1);  
      Polynomial P(VP.begin(), VP.end());
      assert(3 == internal::check_intervals_real_root_isolator<Isolator>(P));
    }{       
        // std::cout << "Wilkinson Polynomial\n";
        int number_of_roots = 20;
        Polynomial P(1);
        for(int i=1; i<=number_of_roots ; i++) {
            P*=Polynomial(NT(-i),NT(1));
        }
        Isolator isolator(P);
        int n = internal::check_intervals_real_root_isolator<Isolator>(P);
        assert( n == number_of_roots);             
    }{
        //std::cout << "Kameny 3\n";
        // from http://www-sop.inria.fr/saga/POL/BASE/1.unipol
 
        NT c = CGAL::ipower(NT(10),12);
        Polynomial P(NT(-3),NT(0),c);
        P = P*P;   // (c^2x^2-3)^2
        Polynomial Q (NT(0),NT(1));
        Q = Q*Q; // x^2
        Q = Q*Q; // x^4
        Q = Q*Q; // x^8
        Q = Q*Polynomial(NT(0),c);//c^2x^9
        P = P+Q;
        
        assert(3 == internal::check_intervals_real_root_isolator<Isolator>(P));
    }{
        //std::cout << "Kameny 4\n";
        // from http://www-sop.inria.fr/saga/POL/BASE/1.unipol
    
        NT z(0);
        NT a = CGAL::ipower(NT(10),24); // a = 10^{24}
       
        Polynomial P(z,NT(4),CGAL::ipower(a,2),z,z,2*a,z,z,NT(1)); 
        // x^8+2*10^{24}*x^5+10^{48}*x^2+4*x  
        P = P * Polynomial(z,z,z,z,z,z,NT(1));
        // x^{14}+2*10^{24}*x^{11}+10^{48}*x^8+4*x^7
        P = P + Polynomial(NT(4),z,z,z,-4*a);
        // x^{14}+2*10^{24}*x^{11}+10^{48}*x^8+4*x^7-4*10^{24}*X^4+4
    
        Isolator isolator(P);
        assert( 4 == internal::check_intervals_real_root_isolator<Isolator>(P));
    }{
        //std::cout << "Polynomial with large and small clustered roots\n";
        // from http://www-sop.inria.fr/saga/POL/BASE/1.unipol
        // there seems to be some error or misunderstanding
        
        NT z(0);
        NT a = CGAL::ipower(NT(10),20); // a = 10^{20}
        
        Polynomial P(z,z,z,z,z,z,z,z,NT(1)); //x^8
        P = P*Polynomial(z,z,z,z,NT(1)); // x^{12}
        Polynomial R(NT(-1),a); // ax-1
        R = R*R;
        R = R*R; // (ax-1)^4
        P = P-R; // x^{12} - (ax-1)^4
        
        assert( 4 == internal::check_intervals_real_root_isolator<Isolator>(P));
    }

#if CGAL_TEST_REAL_ROOT_INTERVAL_ISOLATOR
    {
      assert( 0 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(0), Bound(1)));
      assert( 1 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(0), Bound(2)));
      assert( 1 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(1), Bound(2)));
      assert( 0 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(2), Bound(3)));
      assert( 0 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(-1), Bound(0)));
      assert( 1 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(-2), Bound(0)));
      assert( 1 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(-2), Bound(-1)));
      assert( 0 == internal::check_intervals_real_root_isolator<Isolator>(P_s2, Bound(-3), Bound(-2))); 
    }
#endif
    
}

} //namespace internal

} //namespace CGAL

#endif // CGAL_TEST_REAL_ROOT_ISOLATOR_H
