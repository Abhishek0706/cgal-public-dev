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

/*! \file NiX/Algebraic_real_pure.C
  This is the test file for the class NiX::Algebraic_real_pure. 
*/

#include <CGAL/basic.h>

#include <cassert>

#include <CGAL/Arithmetic_kernel.h>

#include <CGAL/Algebraic_kernel_d/Algebraic_real_pure.h>
#include <CGAL/Algebraic_kernel_d/Algebraic_real_rep.h>
#include <CGAL/Algebraic_kernel_d/Algebraic_real_rep_bfi.h>
#include <CGAL/Algebraic_kernel_d/Algebraic_real_quadratic_refinement_rep_bfi.h>
#include <CGAL/Polynomial_traits_d.h>
#include <CGAL/Sqrt_extension.h>

#include <CGAL/Test/_test_real_embeddable.h>

#include <cstdlib>

/*
COEFF Coefficient type of Polynomial
REAL  a FieldWithSqrt
RATIONAL a numbertype representing the rational numbers
Z a numbertype representing Z (needed for Descartes)
*/
template <class COEFF, class RATIONAL, class Z, class RepClass>
void algebraic_number_test()
{
    typedef COEFF Coeff_NT;
    typedef RATIONAL rat_NT;
    typedef Z Integer;
    
    typedef CGAL::internal::Algebraic_real_pure<Coeff_NT,rat_NT, CGAL::Handle_policy_no_union, RepClass > Algebraic_real_1; 
    typedef typename CGAL::Polynomial_type_generator<Coeff_NT,1>::Type Poly;
    CGAL::test_real_embeddable<Algebraic_real_1>();
    // general test of comparable functionality  

    // TODO generates a precondition error in Algebraic_real_rep
    //NiX::test_real_comparable<Algebraic_real_1>();

    // test of constructors
    Poly P_00(Coeff_NT(0));                   // zero polynomial
    Poly P_01(Coeff_NT(1));                   // constant polynomial
    Poly P_1(Coeff_NT(-1),Coeff_NT(1));       //(x-1)
    Poly P_2(Coeff_NT(-2),Coeff_NT(1));       //(x-2)
    Poly P_3(Coeff_NT(-3),Coeff_NT(1));       //(x-3)
    Poly P_4(Coeff_NT(-4),Coeff_NT(1));       //(x-4)
    Poly P_12=P_1*P_2;    //(x-1)(x-2)
    Poly P_123=P_1*P_2*P_3;    //(x-1)(x-2)(x-3)
    Poly P_s2(Coeff_NT(-2),Coeff_NT(0),Coeff_NT(1)); //(x^2-2)
    Poly P_s3(Coeff_NT(-3),Coeff_NT(0),Coeff_NT(1)); //(x^2-3)
    Poly P_s5(-Coeff_NT(5),Coeff_NT(0),Coeff_NT(1)); 
    Poly P_s10(-Coeff_NT(10),Coeff_NT(0),Coeff_NT(1));
    Poly P_s30(-Coeff_NT(30),Coeff_NT(0),Coeff_NT(1));
    Poly P_s2510= P_s2*P_s5*P_s10;
    Poly P_s530= P_s5 * P_s30;
   
    Algebraic_real_1 tmp;
    Algebraic_real_1 tmp1,tmp2;    

    rat_NT m;
    // general constructors;
    // default 
    // tmp = IS_RATIONAL = 0
    tmp = Algebraic_real_1();
    assert(tmp.is_rational());
    assert(tmp.rational()==0); 
    // from int 
    tmp = Algebraic_real_1(1);
    assert(tmp.is_rational());
    assert(tmp.rational()==1);

    tmp = Algebraic_real_1(5);
    assert(tmp.is_rational());
    assert(tmp.rational()==5);
    
    // from Field
    tmp = Algebraic_real_1(rat_NT(0));
    assert(tmp.is_rational());
    assert(tmp.rational()==0); 
    
    tmp = Algebraic_real_1(rat_NT(1));
    assert(tmp.is_rational());
    assert(tmp.rational()==1);

    tmp = Algebraic_real_1(rat_NT(5)/ rat_NT(2));
    assert(tmp.is_rational());
    assert(tmp.rational()== rat_NT(5)/ rat_NT(2));    

    // general constructor 
    // tmp = 1
#if 0
    tmp = Algebraic_real_1(P_1,-2,+2);
    // TODO different behavior with leda and core
    assert(!tmp.is_rational());
    assert(tmp==rat_NT(1));
    assert(tmp.is_rational());
    assert(tmp.rational()==1);
#endif

    // special constructors 
    // from int
    tmp = Algebraic_real_1(2);
    assert(tmp.is_rational());
    assert(tmp.rational()==rat_NT(2)); 
    //from rat_NT
    tmp = Algebraic_real_1(rat_NT(2));
    assert(tmp.is_rational());
    assert(tmp.rational()==rat_NT(2)); 

    // member functions
    // tmp IS_GENERAL == 2;  

    tmp = Algebraic_real_1(P_123,rat_NT(3)/2,rat_NT(5)/2);
    assert(!tmp.is_rational());
    assert(tmp.polynomial()==P_123);
    assert(tmp.low()==rat_NT(3)/2);
    assert(tmp.high()==rat_NT(5)/2);
    assert(tmp.sign_at_low()==P_123.sign_at(rat_NT(3)/2));  
    
    // refine
    tmp = Algebraic_real_1(P_123,rat_NT(3)/2,rat_NT(5)/2);
    tmp.refine();
    assert(tmp.is_rational());
    assert(tmp.rational()==rat_NT(2));
    // tmp IS_GENERAL = sqrt 2
    tmp = Algebraic_real_1(P_s2*P_3,rat_NT(1),rat_NT(2));
    tmp.refine();
    assert(tmp.low()  >= rat_NT(1)); 
    assert(tmp.high() <= rat_NT(3)/2);   
    
    // strong_refine
    // tmp IS_GENERAL == 2;  
    
    tmp = Algebraic_real_1(P_123,rat_NT(3)/2,rat_NT(5)/2);
    m = rat_NT(2);
    tmp.strong_refine(m);
    assert(tmp.is_rational());
    assert(tmp.rational()==rat_NT(2));
    // tmp IS_GENERAL = sqrt 2
    tmp = Algebraic_real_1(P_s2*P_3,rat_NT(1),rat_NT(2));
    m = rat_NT(3)/2;
    tmp.strong_refine(m);
    assert(tmp.low()!=m);      
    assert(tmp.high()!=m); 
    
    // refine_to(a,b)
    // tmp IS_GENERAL = sqrt 2
    tmp = Algebraic_real_1(P_s2*P_4,rat_NT(0),rat_NT(3));
    assert(!tmp.is_rational());
    tmp.refine_to(rat_NT(1), rat_NT(2));
    assert(tmp.low()  >= rat_NT(1));
    assert(tmp.high() <= rat_NT(2));

    // tmp IS_REAL = sqrt 2
    tmp = Algebraic_real_1(P_s2,rat_NT(0),rat_NT(3));
    assert(!tmp.is_rational());
    tmp.refine_to(rat_NT(1), rat_NT(2));
    assert(tmp.low()  >= rat_NT(1));
    assert(tmp.high() <= rat_NT(2));

    // compare(rat)
    // tmp IS_GENERAL = sqrt 2
    tmp = Algebraic_real_1(P_s2*P_3,rat_NT(1),rat_NT(2));
    m = rat_NT(1);
    assert(tmp.compare(m)==1);
    m = rat_NT(2);
    assert(tmp.compare(m)==-1);
    // tmp IS_GENERAL = 3
    tmp = Algebraic_real_1(P_s2*P_3,rat_NT(2),rat_NT(4));
    m = rat_NT(3);
    assert(tmp.compare(m)==0);
    assert(tmp.is_rational());
    assert(tmp.rational()==rat_NT(3));
    assert(CGAL::degree(tmp.polynomial()) == 1);
    assert(tmp.polynomial().evaluate(Coeff_NT(3)) == Coeff_NT(0));
    
    // compare_distinct()
    
    tmp1 = Algebraic_real_1(P_s530, rat_NT(2), rat_NT(3)); // sqrt(5)  = 2.236...
    tmp2 = Algebraic_real_1(P_s530, rat_NT(5), rat_NT(6)); // sqrt(30) = 5.477...
    assert(tmp1.compare_distinct(tmp2) == CGAL::SMALLER);
    assert(tmp2.compare_distinct(tmp1) == CGAL::LARGER);

    //member functions
    // is_root_of
    tmp1 = Algebraic_real_1(P_s2510,rat_NT(1)/2,rat_NT(3)/2); 
    assert(tmp1.is_root_of(P_s530*P_s2));
    tmp1 = Algebraic_real_1(P_s2510,rat_NT(1)/2,rat_NT(3)/2); 
    assert(!tmp1.is_root_of(P_s530));

    //rational_between
    {
        rat_NT r;
        tmp1 = Algebraic_real_1(P_s2,rat_NT(1),rat_NT(2)); //sqrt2
        tmp2 = Algebraic_real_1(P_s3,rat_NT(1),rat_NT(3)); //sqrt3
        r = tmp1.rational_between(tmp2);
        assert(tmp1.compare(r)==CGAL::SMALLER);
        assert(tmp2.compare(r)==CGAL::LARGER);
        
        r = tmp2.rational_between(tmp1);
        assert(tmp1.compare(r)==CGAL::SMALLER);
        assert(tmp2.compare(r)==CGAL::LARGER);
    }

    // to_double()
    tmp = Algebraic_real_1(P_1*P_3*P_4, rat_NT(0), rat_NT(2));
    assert(fabs(tmp.to_double() - 1.0) < 1e-10);
    tmp = Algebraic_real_1(P_1*P_3, rat_NT(0), rat_NT(2));
    assert(fabs(tmp.to_double() - 1.0) < 1e-10);
    tmp = Algebraic_real_1(P_1, rat_NT(0), rat_NT(2));
    assert(fabs(tmp.to_double() - 1.0) < 1e-10);

    // input/output 
    std::ofstream ofilestream;
    std::ifstream ifilestream;

    // tmp IS_GENERAL = sqrt 2
    tmp1 = Algebraic_real_1(P_s2*P_3,rat_NT(1),rat_NT(2));
    // TODO: No stream operators defined yet!
    /*ofilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::out);
    ofilestream << tmp1;
    ofilestream.close();
   
    ifilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::in);
    ifilestream >> tmp2;
    ifilestream.close();
   
    assert( tmp1 == tmp2 );*/

    //from rat_NT 
    tmp1 = Algebraic_real_1(rat_NT(2));
    // TODO: No stream operators defined yet!  
    /*ofilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::out);
    ofilestream << tmp1;
    ofilestream.close();*/  
 
#if 0
    // TODO generates a precondition error in Algebraic_real_rep    
    ifilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::in);
    ifilestream >> tmp2;
    ifilestream.close();
  
    assert( tmp1 == tmp2 );    
#endif

    // tmp IS_REAL == -sqrt(2);  
    tmp1 = Algebraic_real_1(P_s2,-2,-1);
    // TODO: no stream operators defined yet!
    /*ofilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::out);
    ofilestream << tmp1;
    ofilestream.close(); 
   
    ifilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::in);
    ifilestream >> tmp2;
    ifilestream.close();   
   
    assert( tmp1 == tmp2 );*/

    //test rational input
    // TODO: input and output not supported yet    
    /*ofilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::out);
    ofilestream << NiX::IS_RATIONAL <<" " << RATIONAL(2)<< " ";
    ofilestream.close(); 
   
    ifilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::in);
    ifilestream >> tmp2;
    ifilestream.close();   
    assert( Algebraic_real_1(2) == tmp2 );*/

    /*  ifilestream.open("interval_approx_tmp.txt",std::ios::in);
    cout <<"start reading" << endl;
    ifilestream >> tmp2;
    cout <<"end   reading" << endl;
    ifilestream.close();
    */
    Algebraic_real_1 a1,b1,c1,a2,b2,c2;
    a1 = Algebraic_real_1(P_s2*P_3,rat_NT(1),rat_NT(2));
    b1 = Algebraic_real_1(rat_NT(2));  
    c1 = Algebraic_real_1(P_s2,-2,-1);  
    // TODO: No stream operators defined yet!
    /*ofilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::out);
    ofilestream << a1 << b1 << c1 << a1;
    ofilestream.close();*/  
#if 0
    // TODO generates a precondition error in Algebraic_real_rep
    ifilestream.open("Algebraic_real_pure_stream.test_tmp",std::ios::in);
    ifilestream >> a2 >> b2 >> c2 >> a2;
    ifilestream.close();
    assert( a1 == a2 );
    assert( b1 == b2 );
    assert( c1 == c2 );
#endif
    // test for Handle with union 
    {
        typedef 
            CGAL::internal::Algebraic_real_pure
            <Coeff_NT,rat_NT,::CGAL::Handle_policy_union> Int;
        Int i(5);
        Int j(5);
        Int k(6);
        assert( ! i.identical( j));
        assert( ! i.identical( k));
        assert( ! j.identical( k));
        assert( i == j);
        assert( ! (i == k));
        assert( i.identical( j));
        assert( ! i.identical( k));
        assert( ! j.identical( k));
        // code coverage 
        assert( i == j);
    }
    // test for Handle without union 
    {
        typedef 
            CGAL::internal::Algebraic_real_pure
            <Coeff_NT,rat_NT,::CGAL::Handle_policy_no_union> Int;
        Int i(5);
        Int j(5);
        Int k(6);
        assert( ! i.identical( j));
        assert( ! i.identical( k));
        assert( ! j.identical( k));
        assert( i == j);
        assert( ! (i == k));
        assert( ! i.identical( j));
        assert( ! i.identical( k));
        assert( ! j.identical( k));
    }

    
//     to_interval
//     {
//       Algebraic_real_1 TMP;              
//       assert(CGAL::in(25.0,CGAL::to_interval(Algebraic_real_1(25))));
//       assert(CGAL::in(sqrt(2),CGAL::to_interval(Algebraic_real_1(P_s2,1,2))));
//       assert(CGAL::in(sqrt(2),CGAL::to_interval(Algebraic_real_1(P_s2510,1,2))));
//       assert(CGAL::in(-sqrt(2),CGAL::to_interval(Algebraic_real_1(P_s2510,-2,-1))));
//       assert(CGAL::in(sqrt(5),CGAL::to_interval(Algebraic_real_1(P_s2510,2,3))));
//       assert(CGAL::in(-sqrt(5),CGAL::to_interval(Algebraic_real_1(P_s2510,-3,-2))));
//       assert(CGAL::in(sqrt(10),CGAL::to_interval(Algebraic_real_1(P_s2510,3,4))));
//       assert(CGAL::in(-sqrt(10),CGAL::to_interval(Algebraic_real_1(P_s2510,-4,-3))));
//     } 

    //simplify
    {
        // just a synatx check
        Algebraic_real_1(P_s2510,1,2).simplify();
    }
}

template<class AT> 
void algebraic_number_test_at(){
  typedef typename AT::Integer Integer;
  typedef typename AT::Rational Rational;
  typedef typename CGAL::Sqrt_extension<Integer,Integer> Ext_int_int;
  typedef typename CGAL::Sqrt_extension<Rational,Integer> Ext_rat_int;
  typedef typename CGAL::Sqrt_extension<Rational,Rational> Ext_rat_rat;

/*
  COEFF Coefficient type of Polynomial
  REAL  a FieldWithSqrt
  RATIONAL a numbertype representing the rational numbers
  Z a numbertype representing Z (needed for Descartes)
  template< class COEFF, class REAL, class RATIONAL, class Z>
  algebraic_number_test()
  algebraic_number_test< class COEFF, class REAL, class RATIONAL, class Z>()
*/ 

  typedef CGAL::internal::Algebraic_real_rep< Integer,     Rational>  Rep_int;
  typedef CGAL::internal::Algebraic_real_rep< Rational,    Rational > Rep_rat;
  typedef CGAL::internal::Algebraic_real_rep< Ext_int_int, Rational > Rep_ext_int_int;
  typedef CGAL::internal::Algebraic_real_rep< Ext_rat_int, Rational > Rep_ext_rat_int;
  typedef CGAL::internal::Algebraic_real_rep< Ext_rat_rat, Rational > Rep_ext_rat_rat;
  

  algebraic_number_test<Integer, Rational, Integer,     Rep_int>();
  algebraic_number_test<Rational, Rational, Integer,    Rep_rat>();
  algebraic_number_test<Ext_int_int, Rational, Integer, Rep_ext_int_int>();
  algebraic_number_test<Ext_rat_int, Rational, Integer, Rep_ext_rat_int>();
  algebraic_number_test<Ext_rat_rat, Rational, Integer, Rep_ext_rat_rat>();


  typedef CGAL::internal::Algebraic_real_rep_bfi< Integer,     Rational>  Rep_bfi_int;
  typedef CGAL::internal::Algebraic_real_rep_bfi< Rational,    Rational > Rep_bfi_rat;
  typedef CGAL::internal::Algebraic_real_rep_bfi< Ext_int_int, Rational > Rep_bfi_ext_int_int;
  typedef CGAL::internal::Algebraic_real_rep_bfi< Ext_rat_int, Rational > Rep_bfi_ext_rat_int;
  typedef CGAL::internal::Algebraic_real_rep_bfi< Ext_rat_rat, Rational > Rep_bfi_ext_rat_rat;

  algebraic_number_test<Integer, Rational, Integer,     Rep_bfi_int>();
  algebraic_number_test<Rational, Rational, Integer,    Rep_bfi_rat>();
  algebraic_number_test<Ext_int_int, Rational, Integer, Rep_bfi_ext_int_int>();
  algebraic_number_test<Ext_rat_int, Rational, Integer, Rep_bfi_ext_rat_int>();
  algebraic_number_test<Ext_rat_rat, Rational, Integer, Rep_bfi_ext_rat_rat>();


//  Algebraic_real_quadratic_refinement_rep_bfi
  typedef CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi< Integer,     Rational>  Rep_qr_bfi_int;
  typedef CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi< Rational,    Rational > Rep_qr_bfi_rat;
  typedef CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi< Ext_int_int, Rational > Rep_qr_bfi_ext_int_int;
  typedef CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi< Ext_rat_int, Rational > Rep_qr_bfi_ext_rat_int;
  typedef CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi< Ext_rat_rat, Rational > Rep_qr_bfi_ext_rat_rat;

  algebraic_number_test<Integer, Rational, Integer,     Rep_qr_bfi_int>();
  algebraic_number_test<Rational, Rational, Integer,    Rep_qr_bfi_rat>();
  algebraic_number_test<Ext_int_int, Rational, Integer, Rep_qr_bfi_ext_int_int>();
  algebraic_number_test<Ext_rat_int, Rational, Integer, Rep_qr_bfi_ext_rat_int>();
  algebraic_number_test<Ext_rat_rat, Rational, Integer, Rep_qr_bfi_ext_rat_rat>();

}

int main()
{ 
#ifdef CGAL_HAVE_LEDA_ARITHMETIC_KERNEL
  typedef CGAL::LEDA_arithmetic_kernel LEDA_AK; 
  algebraic_number_test_at<LEDA_AK>();
  std::cerr << " LEDA test ..  " << std::flush;
  std::cerr << " done " << std::endl;
#else
  std::cerr << " LEDA test skipped " << std::endl;
#endif // CGAL_HAVE_LEDA_ARITHMETIC_KERNEL

#ifdef CGAL_HAVE_CORE_ARITHMETIC_KERNEL
  std::cerr << " CORE test ..  " << std::flush;
  typedef CGAL::CORE_arithmetic_kernel CORE_AK; 
  algebraic_number_test_at<CORE_AK>();
  std::cerr << " done " << std::endl;
#else
  std::cerr << " CORE test skipped " << std::endl;
#endif // CGAL_HAVE_CORE_ARITHMETIC_KERNEL

#ifdef CGAL_HAVE_GMP_ARITHMETIC_KERNEL
  std::cerr << " GMP test ..  " << std::flush;
  typedef CGAL::GMP_arithmetic_kernel GMP_AK; 
  algebraic_number_test_at<GMP_AK>(); 
  std::cerr << " done " << std::endl;
#else
  std::cerr << " GMP test skipped " << std::endl;
#endif // CGAL_HAVE_GMP_ARITHMETIC_KERNEL

}
//EOF
