// Copyright (c) 2011 National and Kapodistrian University of Athens (Greece).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author: Luis Peñaranda <luis.penaranda@gmx.com>

// This file contains the simplest refiner, that bisects the interval a given
// number of times.

#ifndef CGAL_RS_SIMPLE_BISECTION_REFINER_1_H
#define CGAL_RS_SIMPLE_BISECTION_REFINER_1_H

#include <CGAL/Polynomial_traits_d.h>
#include "simple_signat_1.h"

namespace CGAL{

template <class Polynomial_,class Bound_>
struct Simple_bisection_refiner_1{
        typedef CGAL::SimpleAK1::Simple_signat_1<Polynomial_,Bound_>    Signat;
        void operator()(const Polynomial_&,Bound_&,Bound_&,int);
}; // class Simple_bisection_refiner_1

// TODO: Write in a generic way, if possible.
template <class Polynomial_,class Bound_>
void
Simple_bisection_refiner_1<Polynomial_,Bound_>::
operator()(const Polynomial_&,Bound_&,Bound_&,int){
        CGAL_error_msg("bisection refiner not implemented for these types");
        return;
};

// TODO: Optimize this function.
template<>
void
Simple_bisection_refiner_1<Polynomial<Gmpz>,Gmpfr>::
operator()(const Polynomial<Gmpz> &pol,Gmpfr &left,Gmpfr &right,int prec){
        typedef Polynomial<Gmpz>                        Polynomial;
        typedef Polynomial_traits_d<Polynomial>         Ptraits;
        typedef Ptraits::Degree                         Degree;
        typedef Ptraits::Make_square_free               Sfpart;
        typedef CGAL::SimpleAK1::Simple_signat_1<Polynomial,Gmpfr>
                                                        Signat;
        CGAL_precondition(left<=right);
        // TODO: add precondition to check whether the interval is a point
        // or the evaluations on its endpoints have different signs
        //std::cout<<"refining ["<<left<<","<<right<<"]"<<std::endl;
        // Check first that endpoints are unique.
        if(!left.is_unique()){
                mpfr_t new_left_mpfr;
                mpfr_init2(new_left_mpfr,left.get_precision());
                mpfr_set(new_left_mpfr,left.fr(),GMP_RNDN);
                CGAL_assertion(mpfr_equal_p(new_left_mpfr,left.fr()));
                left=Gmpfr(new_left_mpfr);
                CGAL_assertion(left.is_unique());
        }
        if(!right.is_unique()){
                mpfr_t new_right_mpfr;
                mpfr_init2(new_right_mpfr,right.get_precision());
                mpfr_set(new_right_mpfr,right.fr(),GMP_RNDN);
                CGAL_assertion(mpfr_equal_p(new_right_mpfr,right.fr()));
                right=Gmpfr(new_right_mpfr);
                CGAL_assertion(right.is_unique());
        }

        Polynomial sfpp=Sfpart()(pol);
        Signat signof(sfpp);
        CGAL::Sign sl,sc;
        mp_prec_t pl,pc;
        mpfr_t center;
        int round;

        sl=signof(left);
        if(sl==ZERO)
                return;
        pl=left.get_precision();
        pc=right.get_precision();
        pc=(pl>pc?pl:pc)+(mp_prec_t)prec;
        mpfr_init2(center,pc);
        round=mpfr_prec_round(left.fr(),pc,GMP_RNDN);
        CGAL_assertion(!round);
        round=mpfr_prec_round(right.fr(),pc,GMP_RNDN);
        CGAL_assertion(!round);
        for(int i=0;i<prec;++i){
                round=mpfr_add(center,left.fr(),right.fr(),GMP_RNDN);
                CGAL_assertion(!round);
                round=mpfr_div_2ui(center,center,1,GMP_RNDN);
                CGAL_assertion(!round);
                sc=signof(Gmpfr(center));
                if(sc==ZERO){   // we have a root
                        round=mpfr_set(left.fr(),center,GMP_RNDN);
                        CGAL_assertion(!round);
                        mpfr_swap(right.fr(),center);
                        break;
                }
                if(sc==sl)
                        mpfr_swap(left.fr(),center);
                else
                        mpfr_swap(right.fr(),center);
        }
        mpfr_clear(center);
        CGAL_postcondition(left<=right);
        //std::cout<<"ref root is ["<<left<<","<<right<<"]"<<std::endl;
        return;
};

} // namespace CGAL

#endif // CGAL_RS_SIMPLE_BISECTION_REFINER_1_H
