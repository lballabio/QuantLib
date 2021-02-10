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

/*! \file analyticcevengine.hpp
    \brief  Pricing engine for European vanilla options using a
    constant elasticity of variance (CEV) model
*/

#ifndef quantlib_analytic_cev_engine_hpp
#define quantlib_analytic_cev_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

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

    class CEVCalculator {
      public:
        CEVCalculator(Real f0, Real alpha, Real beta);

        Real value(Option::Type optionType, Real strike, Time t) const;

        Real f0()    const { return f0_; }
        Real alpha() const { return alpha_; }
        Real beta()  const { return beta_; }

      private:
        Real X(Real f) const;

        const Real f0_, alpha_, beta_, delta_, x0_;
    };


    class AnalyticCEVEngine : public VanillaOption::engine {
      public:
        AnalyticCEVEngine(Real f0, Real alpha, Real beta, Handle<YieldTermStructure> discountCurve);

        void calculate() const override;

      private:
        const ext::shared_ptr<CEVCalculator> calculator_;
        const Handle<YieldTermStructure> discountCurve_;
    };
}

#endif
