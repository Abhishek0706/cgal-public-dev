#include <CGAL/basic.h>

namespace CGAL{
namespace Test{

template <class T, class RootOf1, class RootOf2>
void test_root_of_traits(){
    // pure type checking  
    typedef CGAL::Root_of_traits<T> RoT;
    typedef typename RoT::Root_of_1 Root_of_1;
    typedef typename RoT::Root_of_2 Root_of_2;
    
    BOOST_STATIC_ASSERT((::boost::is_same<RootOf1,Root_of_1>::value));
    BOOST_STATIC_ASSERT((::boost::is_same<RootOf2,Root_of_2>::value));
    
    typedef typename RoT::Make_root_of_2 Make_root_of_2;
    typedef typename RoT::Make_sqrt      Make_sqrt;
    typedef typename RoT::Inverse        Inverse;  
    typedef typename RoT::Square         Square; 

    const Make_root_of_2& make_root_of_2 = Make_root_of_2();
    const Make_sqrt&      make_sqrt      = Make_sqrt();
    const Inverse&        inverse        = Inverse();  
    const Square&         square         = Square(); 
    
    BOOST_STATIC_ASSERT((::boost::is_same<Root_of_2,typename Make_root_of_2::result_type>::value));
    BOOST_STATIC_ASSERT((::boost::is_same<Root_of_2,typename Make_sqrt::result_type>::value));
    BOOST_STATIC_ASSERT((::boost::is_same<Root_of_2,typename Inverse::result_type>::value));
    BOOST_STATIC_ASSERT((::boost::is_same<Root_of_2,typename Square::result_type>::value));


    {
      Root_of_2 r  = make_root_of_2(T(0),T(-1),T(2)); //-sqrt(2)
      Root_of_2 rl = make_root_of_2(T(1),T(0),T(-2),true); //-sqrt(2);
      Root_of_2 rr = make_root_of_2(T(1),T(0),T(-2),false); //+sqrt(2)
      assert(r == rl);
      assert(rl != rr);
      
      assert( r * Root_of_1(2) == make_root_of_2(T(0),T(-2),T(2)));
      assert( r * T(2) == make_root_of_2(T(0),T(-2),T(2)));
    }{
      Root_of_2 r  = CGAL::make_root_of_2(T(0),T(-1),T(2)); //-sqrt(2)
      Root_of_2 rl = CGAL::make_root_of_2(T(1),T(0),T(-2),true); //-sqrt(2);
      Root_of_2 rr = CGAL::make_root_of_2(T(1),T(0),T(-2),false); //+sqrt(2)
      assert(r == rl);
      assert(rl != rr);
      
      assert( r * Root_of_1(2) == CGAL::make_root_of_2(T(0),T(-2),T(2)));
      assert( r * T(2) == CGAL::make_root_of_2(T(0),T(-2),T(2)));
    }

   
    {
      Root_of_2 r  = make_sqrt(T(2)); //sqrt(2)
      Root_of_2 rr = make_root_of_2(T(1),T(0),T(-2),false); //+sqrt(2)
      assert(r == rr);
    }{
      Root_of_2 r  = CGAL::make_sqrt(T(2)); //sqrt(2)
      Root_of_2 rr = CGAL::make_root_of_2(T(1),T(0),T(-2),false); //+sqrt(2)
      assert(r == rr);
    }

    {
      Root_of_2 r  = inverse(CGAL::make_sqrt(T(2))); 
      Root_of_2 rr = 1/CGAL::make_sqrt(T(2));
      assert(r == rr);
    }{
        Root_of_2 r  = CGAL::inverse(CGAL::make_sqrt(T(2)));
        Root_of_2 rr = 1/CGAL::make_sqrt(T(2)); 
        assert(r == rr);
    }

    {
      Root_of_2 r  = square(CGAL::make_sqrt(T(2)));
      Root_of_2 rr = CGAL::make_sqrt(T(2))*CGAL::make_sqrt(T(2));
      assert(r == rr);
    }{
      Root_of_2 r  = CGAL::square(CGAL::make_sqrt(T(2)));
      Root_of_2 rr = CGAL::make_sqrt(T(2))*CGAL::make_sqrt(T(2));
      assert(r == rr);
    }
    
}

} // namespace Test 
} // namespace CGAL 
