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

#ifndef quantlib_normal_clv_model_hpp
#define quantlib_normal_clv_model_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/time/date.hpp>
#include <ql/functional.hpp>

namespace QuantLib {
    /*! References:

        A. Grzelak, 2016, The CLV Framework -
        A Fresh Look at Efficient Pricing with Smile

        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2747541
    */

    class PricingEngine;
    class GBSMRNDCalculator;
    class OrnsteinUhlenbeckProcess;
    class GeneralizedBlackScholesProcess;

    class NormalCLVModel : public LazyObject {
      public:
        NormalCLVModel(const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
                       ext::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess,
                       const std::vector<Date>& maturityDates,
                       Size lagrangeOrder,
                       Real pMax = Null<Real>(),
                       Real pMin = Null<Real>());

        // cumulative distribution function of the BS process
        Real cdf(const Date& d, Real x) const;

        // inverse cumulative distribution function of the BS process
        Real invCDF(const Date& d, Real q) const;

        // collocation points of the Ornstein-Uhlenbeck process
        Disposable<Array> collocationPointsX(const Date& d) const;

        // collocation points for the underlying Y
        Disposable<Array> collocationPointsY(const Date& d) const;

        // CLV mapping function
        ext::function<Real(Time, Real)> g() const;

      protected:
        void performCalculations() const override;

      private:
        class MappingFunction {
          public:
            explicit MappingFunction(const NormalCLVModel& model);

            Real operator()(Time t, Real x) const;

          private:
            mutable Array y_;
            const Volatility sigma_;
            const ext::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess_;

            struct InterpolationData {
                explicit InterpolationData(const NormalCLVModel& model)
                : s_(model.x_.size(), model.maturityDates_.size()),
                  x_(model.x_),
                  t_(model.maturityTimes_),
                  lagrangeInterpl_(x_.begin(), x_.end(), x_.begin()) {}

                Matrix s_;
                std::vector<LinearInterpolation> interpl_;

                const Array x_;
                const std::vector<Time> t_;
                const LagrangeInterpolation lagrangeInterpl_;
            };

            const ext::shared_ptr<InterpolationData> data_;
        };


        const Array x_;
        const Volatility sigma_;
        const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        const ext::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess_;
        const std::vector<Date> maturityDates_;
        const ext::shared_ptr<GBSMRNDCalculator> rndCalculator_;

        std::vector<Time> maturityTimes_;
        mutable ext::function<Real(Time, Real)> g_;
    };
}

#endif


#ifndef id_d5d293d8e006f8b5d316bbe61ad47c98
#define id_d5d293d8e006f8b5d316bbe61ad47c98
inline bool test_d5d293d8e006f8b5d316bbe61ad47c98(int* i) { return i != 0; }
#endif
