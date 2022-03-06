/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Hachemi Benyahia
 Copyright (C) 2010 DeriveXperts SAS

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file huslerreisscopula.hpp
    \brief Husler-Reiss copula
*/

#ifndef quantlib_math_husler_reiss_copula_hpp
#define quantlib_math_husler_reiss_copula_hpp

#include <ql/math/distributions/normaldistribution.hpp>
#include <functional>

namespace QuantLib {

    //! Husler-Reiss copula
    class HuslerReissCopula {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;

        HuslerReissCopula(Real theta_);
        Real operator()(Real x, Real y) const;
      private:
        Real theta_;
        CumulativeNormalDistribution cumNormal_;
    };

}

#endif


#ifndef id_d4a43391cad6e23765a7ad7383130dff
#define id_d4a43391cad6e23765a7ad7383130dff
inline bool test_d4a43391cad6e23765a7ad7383130dff(const int* i) {
    return i != nullptr;
}
#endif
