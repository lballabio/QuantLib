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

/*! \file normalclvmodel.hpp
    \brief CLV model with a normally distributed kernel process
*/

#ifndef quantlib_normal_clv_mc_model_hpp
#define quantlib_normal_clv_mc_model_hpp

#include <ql/handle.hpp>
#include <ql/exercise.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/processes/blackscholesprocess.hpp>

#include <boost/function.hpp>

namespace QuantLib {
    /*! References:

        A. Grzelak, 2015, The CLV Framework -
        A Fresh Look at Efficient Pricing with Smile

        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2747541
    */

    class PricingEngine;

    class NormalCLVMCModel : public LazyObject {
      public:
        NormalCLVMCModel(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
            const std::vector<Date>& maturityDates);

        void update();

        // cumulative distribution function of the BS process
        Real F(const Date& maturityDate, Real x) const;

        boost::function<Real(Time, Real)> g() const;
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process() const;

      protected:
        void performCalculations() const;

      private:
        const boost::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        const std::vector<Date>& maturityDates_;
        const boost::shared_ptr<PricingEngine> pricingEngine_;
    };
}

#endif
