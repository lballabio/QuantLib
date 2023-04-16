/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

/*! \file analyticvariancegammaengine.hpp
    \brief Analytic Variance Gamma option engine for vanilla options
*/

#ifndef quantlib_variance_gamma_engine_hpp
#define quantlib_variance_gamma_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/experimental/variancegamma/variancegammaprocess.hpp>

namespace QuantLib {

    //! Variance Gamma Pricing engine for European vanilla options using integral approach
    /*! \ingroup vanillaengines

        \test the correctness of the returned values is tested by
        checking it against known good results.
    */
    class VarianceGammaEngine : public VanillaOption::engine {
    public:
      explicit VarianceGammaEngine(std::shared_ptr<VarianceGammaProcess>,
                                   Real absoluteError = 1e-5);
      void calculate() const override;

    private:
      std::shared_ptr<VarianceGammaProcess> process_;
      Real absErr_;
    };

}


#endif

