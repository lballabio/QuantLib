/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file gaussiannoncentralchisquaredpolynomial.hpp
    \brief non central chi squared polynomials for Gaussian quadratures
*/

#ifndef quantlib_gaussian_non_central_chi_squared_polynomial_hpp
#define quantlib_gaussian_non_central_chi_squared_polynomial_hpp

#include <ql/functional.hpp>
#include <ql/math/integrals/momentbasedgaussianpolynomial.hpp>

namespace QuantLib {

    class GaussNonCentralChiSquaredPolynomial
            : public MomentBasedGaussianPolynomial<Real> {
      public:
        GaussNonCentralChiSquaredPolynomial(Real nu, Real lambda);

        Real w(Real x) const override;
        Real moment(Size i) const override;

      private:
        const Real nu_, lambda_;

        static std::vector<ext::function<Real(Real, Real)> > moments;
    };
}

#endif


#ifndef id_9c185121c5bdc0dd2adc351fa46ad394
#define id_9c185121c5bdc0dd2adc351fa46ad394
inline bool test_9c185121c5bdc0dd2adc351fa46ad394(int* i) { return i != 0; }
#endif
