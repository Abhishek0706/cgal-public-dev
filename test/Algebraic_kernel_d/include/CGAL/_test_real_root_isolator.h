// TODO: Add licence
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL:$
// $Id: $
// 
//
// Author(s)     :  
//
// ============================================================================

// TODO: The comments are all original EXACUS comments and aren't adapted. So
//         they may be wrong now.

#include <CGAL/basic.h>
#include <cassert>
#include <vector>

#include <CGAL/ipower.h>
/*#include <NiX/basic.h>
#include <NiX/number_type_utils.h>*/

#ifndef CGAL_TEST_REAL_ROOT_ISOLATOR_H
#define CGAL_TEST_REAL_ROOT_ISOLATOR_H

CGAL_BEGIN_NAMESPACE
  
namespace CGALi {
	
template <class RealRootIsolator>
int check_intervals_real_root_isolator( 
        const typename RealRootIsolator::Polynomial& P) {

    typedef RealRootIsolator Isolator;
    typedef typename Isolator::Bound Bound;
    
    Isolator Isol(P);
    int n = Isol.number_of_real_roots();
    for(int i=0; i<n; i++) {
       Bound left  = Isol.left_bound(i);
       Bound right = Isol.right_bound(i);

       if(!Isol.is_exact_root(i)) {
           assert(left < right);
	       //std::cout << " left = " << left << std::endl;
	       //std::cout << " right = " << right << std::endl;
	       //std::cout << " P = " << P << std::endl;
           assert(P.sign_at(left) * P.sign_at(right) == CGAL::NEGATIVE);
       }else{
           assert(left == right);
           assert(P.sign_at(left) == CGAL::ZERO);
       }
    }
    return n;
};


// Not part of the concept 
/*
  template <class RealRootIsolator, class Polynomial, class Bound>
  int check_intervals_real_root_isolator( const Polynomial& P, 
  const Bound& a,
  const Bound& b) {
  
  typedef RealRootIsolator Isolator;
  Isolator Isol(P,a,b);
  int n = Isol.number_of_real_roots();
  for(int i=0; i<n; i++) {
  Bound left = Isol.left_bound(i);
  Bound right = Isol.right_bound(i);
  
  assert( left < right || Isol.is_exact_root(i));
  if(!Isol.is_exact_root(i)) {
  //std::cout << " left = " << left << std::endl;
  //std::cout << " right = " << right << std::endl;
  //std::cout << " P = " << P << std::endl;
  assert(P.evaluate(left) * P.evaluate(right) < 0);
  }
  }
  return n;
  };
*/


template <class RealRootIsolator>
void test_real_root_isolator() {
    typedef RealRootIsolator Isolator;
    typedef typename Isolator::Polynomial    Polynomial;
    typedef typename Isolator::Bound Bound;
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
    }{
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
    {  
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P_123);
        assert( n == 3);
       
    }{  
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P_1);
        assert( n == 1);
       
    }{  
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P_123);
        assert( n == 3);
       
    }{  
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P_s2510);
        assert( n == 6);
       
    }{  // (x^2-2)*(x^2-3)
        std::vector<NT> VP(5);
        VP[0] = NT(6);
        VP[1] = NT(0);
        VP[2] = NT(-5);
        VP[3] = NT(0);
        VP[4] = NT(1);  
        Polynomial P(VP.begin(), VP.end()); 
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
        assert( n == 4);
       
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
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
        assert( n == 3);
    }{       
        // std::cout << "Wilkinson Polynomial\n";
        int number_of_roots = 20;
        Polynomial P(1);
        for(int i=1; i<=number_of_roots ; i++) {
            P*=Polynomial(NT(-i),NT(1));
        }
        Isolator isolator(P);
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
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
        
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
        assert(n == 3);
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
    
        Isolator isol(P);
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
        assert( n == 4);
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
        
        int n = CGALi::check_intervals_real_root_isolator<Isolator>(P);
        assert( n == 4);  
    }
    
};

} //namespace CGALi

CGAL_END_NAMESPACE

#endif // CGAL_TEST_REAL_ROOT_ISOLATOR_H
