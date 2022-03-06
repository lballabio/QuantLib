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

/*! \file squarerootclvmodel.hpp
    \brief CLV model with a square root kernel process
*/

#ifndef quantlib_square_root_clv_model_hpp
#define quantlib_square_root_clv_model_hpp

#include <ql/time/date.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/experimental/math/gaussiannoncentralchisquaredpolynomial.hpp>
#include <ql/functional.hpp>
#include <map>

namespace QuantLib {

    class GBSMRNDCalculator;
    class SquareRootProcess;
    class GeneralizedBlackScholesProcess;

    class SquareRootCLVModel : public LazyObject {
      public:
        SquareRootCLVModel(const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
                           ext::shared_ptr<SquareRootProcess> sqrtProcess,
                           std::vector<Date> maturityDates,
                           Size lagrangeOrder,
                           Real pMax = Null<Real>(),
                           Real pMin = Null<Real>());

        // cumulative distribution function of the BS process
        Real cdf(const Date& d, Real x) const;

        // inverse cumulative distribution function of the BS process
        Real invCDF(const Date& d, Real q) const;

        // collocation points of the square root process
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
            explicit MappingFunction(const SquareRootCLVModel& model);

            Real operator()(Time t, Real x) const;

          private:
            const ext::shared_ptr<Matrix> s_, x_;
            typedef std::map<Time, ext::shared_ptr<LagrangeInterpolation> >
                interpl_type;

            interpl_type interpl;
        };

        std::pair<Real, Real> nonCentralChiSquaredParams(const Date& d) const;

        const Real pMax_, pMin_;
        const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        const ext::shared_ptr<SquareRootProcess> sqrtProcess_;
        const std::vector<Date> maturityDates_;
        const Size lagrangeOrder_;
        const ext::shared_ptr<GBSMRNDCalculator> rndCalculator_;

        mutable ext::function<Real(Time, Real)> g_;
    };
}

#endif


#ifndef id_5cbc8439bd8eedaf7dea35218e7c49c2
#define id_5cbc8439bd8eedaf7dea35218e7c49c2
inline bool test_5cbc8439bd8eedaf7dea35218e7c49c2(const int* i) {
    return i != nullptr;
}
#endif
