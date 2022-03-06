/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file cevrndcalculator.hpp
    \brief risk neutral density calculator for the
    constant elasticity of variance (CEV) model
*/

#ifndef quantlib_cev_rnd_calculator_hpp
#define quantlib_cev_rnd_calculator_hpp

#include <ql/methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>

namespace QuantLib {
    //! constant elasticity of variance process (absorbing boundary at f=0)
    /*! \f[
         df_t = \alpha f_t^\beta \mathrm{d}W_t
        \f]
    */

    /*! References:

        D.R. Brecher, A.E. Lindsay, Results on the CEV Process, Past and Present
        https://www.fincad.com/sites/default/files/wysiwyg/Resources-Wiki/cev-process-working-paper.pdf
    */

    class CEVRNDCalculator : public RiskNeutralDensityCalculator {
      public:
        CEVRNDCalculator(Real f0, Real alpha, Real beta);

        Real massAtZero(Time t) const;

        Real pdf(Real f, Time t) const override;
        Real cdf(Real f, Time t) const override;
        Real invcdf(Real q, Time t) const override;

      private:
        Real X(Real f) const;
        Real invX(Real f) const;
        Real sankaranApprox(Real f, Time t, Real x) const;

        const Real f0_, alpha_, beta_, delta_, x0_;
    };
}

#endif


#ifndef id_c2bca5fa7441bd1ad29ac228c1bc25dd
#define id_c2bca5fa7441bd1ad29ac228c1bc25dd
inline bool test_c2bca5fa7441bd1ad29ac228c1bc25dd(const int* i) {
    return i != nullptr;
}
#endif
