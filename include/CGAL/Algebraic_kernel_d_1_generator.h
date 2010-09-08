// TODO: Add licence
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL:$
// $Id: $
// 
//
// Author(s)     : Michael Kerber <mkerber@mpi-inf.mpg.de>
//
// ============================================================================

// This file defines several instances of Algebraic_kernel_d_1 that are often
// used in tests and demos. 

#ifndef CGAL_ALGEBRAIC_KERNEL_1_GENERATOR_H
#define CGAL_ALGEBRAIC_KERNEL_1_GENERATOR_H 1

#include <CGAL/config.h>

#include <CGAL/Arithmetic_kernel.h>
#include <CGAL/Algebraic_kernel_d_1.h>

// Needed for the "fast" kernel
#include <CGAL/Algebraic_kernel_d/Algebraic_real_quadratic_refinement_rep_bfi.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_descartes.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_descartes_rndl_tree_traits.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_coefficient_kernel.h>

// Needed for the bisection kernel
#include <CGAL/Algebraic_kernel_d/Algebraic_real_rep.h>
#include <CGAL/Algebraic_kernel_d/Descartes.h>

#if (defined(CGAL_USE_GMP) && defined(CGAL_USE_MPFI) && defined(CGAL_USE_RS))
#include <CGAL/Algebraic_kernel_d/Real_solve.h>
#endif

namespace CGAL {

/**
 * Defines default and a fast Algebraic_kernel_d_1, 
 * depending on the coefficient type.
 */
template<typename Coefficient, 
         typename Bound = typename CGAL::Get_arithmetic_kernel< Coefficient >::Arithmetic_kernel::Rational >
struct Algebraic_kernel_d_1_generator {

    typedef CGAL::Algebraic_kernel_d_1 <Coefficient, Bound>
        Default_algebraic_kernel_1;

    typedef CGAL::Algebraic_kernel_d_1
    < Coefficient, 
      Bound,
      CGAL::internal::Algebraic_real_rep< Coefficient, Bound >,
      CGAL::internal::Descartes< CGAL::Polynomial< Coefficient >, Bound >
    > Algebraic_kernel_with_bisection_and_descartes_1;

    typedef CGAL::Algebraic_kernel_d_1
    < Coefficient, 
      Bound,
      CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi
           < Coefficient, Bound >,
      CGAL::internal::Bitstream_descartes
        < CGAL::internal::Bitstream_descartes_rndl_tree_traits
            < CGAL::internal::Bitstream_coefficient_kernel<Coefficient> > 
        >
    > Algebraic_kernel_with_qir_and_bitstream_1;

    typedef CGAL::Algebraic_kernel_d_1
    < Coefficient, 
      Bound,
      CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi
           < Coefficient, Bound >,
      CGAL::internal::Descartes< CGAL::Polynomial< Coefficient >, Bound >
    > Algebraic_kernel_with_qir_and_descartes_1;


#if (defined(CGAL_USE_GMP) && defined(CGAL_USE_MPFI) && defined(CGAL_USE_RS))
    typedef CGAL::Algebraic_kernel_d_1
    < Coefficient, 
      Bound,
      CGAL::internal::Algebraic_real_quadratic_refinement_rep_bfi
           < Coefficient, Bound >,
      CGAL::internal::Real_solve< CGAL::Polynomial< Coefficient >, Bound >
    > Algebraic_kernel_with_qir_and_rs_1;
#endif

};

} //namespace CGAL

#endif
