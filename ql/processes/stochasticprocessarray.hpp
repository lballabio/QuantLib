/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file stochasticprocessarray.hpp
    \brief Array of correlated 1-D stochastic processes
*/

#ifndef quantlib_stochastic_process_array_hpp
#define quantlib_stochastic_process_array_hpp

#include <ql/stochasticprocess.hpp>
#include <vector>

namespace QuantLib {

    //! %Array of correlated 1-D stochastic processes
    /*! \ingroup processes */
    class StochasticProcessArray : public StochasticProcess {
      public:
        StochasticProcessArray(
                  const std::vector<ext::shared_ptr<StochasticProcess1D> >&,
                  const Matrix& correlation);
        // stochastic process interface
        Size size() const override;
        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Array expectation(Time t0, const Array& x0, Time dt) const override;

        Matrix diffusion(Time t, const Array& x) const override;
        Matrix covariance(Time t0, const Array& x0, Time dt) const override;
        Matrix stdDeviation(Time t0, const Array& x0, Time dt) const override;

        Array apply(const Array& x0, const Array& dx) const override;
        Array evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        Time time(const Date&) const override;
        // inspectors
        const ext::shared_ptr<StochasticProcess1D>& process(Size i) const;
        Matrix correlation() const;
      protected:
        std::vector<ext::shared_ptr<StochasticProcess1D> > processes_;
        Matrix sqrtCorrelation_;
    };

}


#endif
