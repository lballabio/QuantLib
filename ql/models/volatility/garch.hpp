/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2012 Liquidnet Holdings, Inc.

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

/*! \file garch.hpp
    \brief GARCH volatility model
*/

#ifndef quantlib_garch_volatility_model_hpp
#define quantlib_garch_volatility_model_hpp

#include <ql/volatilitymodel.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <vector>

namespace QuantLib {

    //! GARCH volatility model
    /*! Volatilities are assumed to be expressed on an annual basis.
    */
    class Garch11 : public VolatilityCompositor {
      public:
        typedef TimeSeries<Volatility> time_series;
        typedef time_series::const_iterator const_iterator;
        typedef time_series::const_value_iterator const_value_iterator;

        enum Mode {
            MomentMatchingGuess,   /*!< The initial guess is a moment
                                        matching estimates for
                                        mean(r2), acf(0), and acf(1). */
            GammaGuess,            /*!< The initial guess is an
                                        estimate of gamma based on the
                                        property:
                                        acf(i+1) = gamma*acf(i) for i > 1. */
            BestOfTwo,             /*!< The best of the two above modes */
            DoubleOptimization     /*!< Double optimization */
        };

        //! \name Constructors
        //@{
        Garch11(Real a, Real b, Real vl)
        : alpha_(a), beta_(b), gamma_(1 - a - b),
          vl_(vl), logLikelihood_(0), mode_(BestOfTwo) {}

        Garch11(const time_series& qs, Mode mode = BestOfTwo)
        : alpha_(0), beta_(0), vl_(0), logLikelihood_(0), mode_(mode) {
            calibrate(qs);
        };
        //@}

        //! \name Inspectors
        //@{
        Real alpha() const { return alpha_; }
        Real beta() const { return beta_; }
        Real omega() const { return vl_ * gamma_; }
        Real ltVol() const { return vl_; }
        Real logLikelihood() const { return logLikelihood_; }
        Mode mode() const { return mode_; }
        //@}

        //! \name VolatilityCompositor interface
        //@{
        time_series calculate(const time_series& quoteSeries) override {
            return calculate(quoteSeries, alpha(), beta(), omega());
        }
        void calibrate(const time_series& quoteSeries) override {
            calibrate(quoteSeries.cbegin_values(), quoteSeries.cend_values());
        }
        //@}

        //! \name Additional interface
        //@{
        static time_series calculate(const time_series& quoteSeries,
                                     Real alpha, Real beta, Real omega);

        void calibrate(const time_series& quoteSeries,
                       OptimizationMethod& method,
                       const EndCriteria& endCriteria) {
            calibrate(quoteSeries.cbegin_values(), quoteSeries.cend_values(),
                      method, endCriteria);
        }

        void calibrate(const time_series& quoteSeries,
                       OptimizationMethod& method,
                       const EndCriteria& endCriteria,
                       const Array& initialGuess) {
            calibrate(quoteSeries.cbegin_values(), quoteSeries.cend_values(),
                      method, endCriteria, initialGuess);
        }

        template <typename ForwardIterator>
        void calibrate(ForwardIterator begin, ForwardIterator end) {
            std::vector<Volatility> r2;
            Real mean_r2 = to_r2(begin, end, r2);
            ext::shared_ptr<Problem> p =
                calibrate_r2(mode_, r2, mean_r2, alpha_, beta_, vl_);
            gamma_ = 1 - alpha_ - beta_;
            vl_ /= gamma_;
            logLikelihood_ = p ? -p->functionValue() :
                                 -costFunction(begin, end);
        }

        template <typename ForwardIterator>
        void calibrate(ForwardIterator begin, ForwardIterator end,
                       OptimizationMethod& method,
                       EndCriteria endCriteria) {
            std::vector<Volatility> r2;
            Real mean_r2 = to_r2(begin, end, r2);
            ext::shared_ptr<Problem> p =
                calibrate_r2(mode_, r2, mean_r2, method,
                             endCriteria, alpha_, beta_, vl_);
            gamma_ = 1 - alpha_ - beta_;
            vl_ /= gamma_;
            logLikelihood_ = p ? -p->functionValue() :
                                 -costFunction(begin, end);
        }

        template <typename ForwardIterator>
        void calibrate(ForwardIterator begin, ForwardIterator end,
                       OptimizationMethod& method,
                       EndCriteria endCriteria,
                       const Array& initialGuess) {
            std::vector<Volatility> r2;
            to_r2(begin, end, r2);
            ext::shared_ptr<Problem> p =
                calibrate_r2(r2, method, endCriteria, initialGuess,
                             alpha_, beta_, vl_);
            gamma_ = 1 - alpha_ - beta_;
            vl_ /= gamma_;
            logLikelihood_ = p ? -p->functionValue() :
                                 -costFunction(begin, end);
        }

        Real forecast(Real r, Real sigma2) const {
            return gamma_* vl_ + alpha_ * r * r + beta_ * sigma2;
        }

        // a helper for calculation of r^2 and <r^2>
        template <typename InputIterator>
        static Real to_r2(InputIterator begin, InputIterator end,
                          std::vector<Volatility>& r2) {
            Real u2(0.0), mean_r2(0.0), w(1.0);
            for (; begin != end; ++begin) {
                u2 = *begin; u2 *= u2;
                mean_r2 = (1.0 - w) * mean_r2 + w * u2;
                r2.push_back(u2);
                w /= (w + 1.0);
            }
            return mean_r2;
        }

        /*! calibrates GARCH for r^2 */
        static ext::shared_ptr<Problem> calibrate_r2(
                                        Mode mode,
                                        const std::vector<Volatility>& r2,
                                        Real mean_r2,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        /*! calibrates GARCH for r^2 with user-defined optimization
            method and end criteria */
        static ext::shared_ptr<Problem> calibrate_r2(
                                        Mode mode,
                                        const std::vector<Volatility>& r2,
                                        Real mean_r2,
                                        OptimizationMethod& method,
                                        const EndCriteria& endCriteria,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        /*! calibrates GARCH for r^2 with user-defined optimization
            method, end criteria and initial guess */
        static ext::shared_ptr<Problem> calibrate_r2(
                                        const std::vector<Volatility>& r2,
                                        Real mean_r2,
                                        OptimizationMethod& method,
                                        const EndCriteria& endCriteria,
                                        const Array& initialGuess,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        /*! calibrates GARCH for r^2 with user-defined optimization
            method, end criteria and initial guess */
        static ext::shared_ptr<Problem> calibrate_r2(
                                        const std::vector<Volatility> &r2,
                                        OptimizationMethod& method,
                                        const EndCriteria& endCriteria,
                                        const Array& initialGuess,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        /*! calibrates GARCH for r^2 with user-defined optimization
            method, end criteria, constraints and initial guess */
        static ext::shared_ptr<Problem> calibrate_r2(
                                        const std::vector<Volatility>& r2,
                                        Real mean_r2,
                                        OptimizationMethod& method,
                                        Constraint& constraints,
                                        const EndCriteria& endCriteria,
                                        const Array& initialGuess,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        static ext::shared_ptr<Problem> calibrate_r2(
                                        const std::vector<Volatility> &r2,
                                        OptimizationMethod& method,
                                        Constraint& constraints,
                                        const EndCriteria& endCriteria,
                                        const Array& initialGuess,
                                        Real& alpha,
                                        Real& beta,
                                        Real& omega);

        template<class InputIterator>
        static Real costFunction(InputIterator begin, InputIterator end,
                                 Real alpha, Real beta, Real omega) {
            Real retval(0.0);
            Real u2(0.0), sigma2(0.0);
            Size N = 0;
            for (; begin != end; ++begin, ++N) {
                sigma2 = omega + alpha * u2 + beta * sigma2;
                u2 = *begin; u2 *= u2;
                retval += std::log(sigma2) + u2 / sigma2;
            }
            return N > 0 ? retval / (2*N) : 0.0;
        }
        //@}
      private:
        Real alpha_, beta_, gamma_, vl_;
        Real logLikelihood_;
        Mode mode_;

        template<class InputIterator>
        Real costFunction(InputIterator begin, InputIterator end) const {
            return costFunction(begin, end, alpha(), beta(), omega());
        }
    };

}


#endif


#ifndef id_7e43b4c5787b54edafc768015d50ea02
#define id_7e43b4c5787b54edafc768015d50ea02
inline bool test_7e43b4c5787b54edafc768015d50ea02(int* i) { return i != 0; }
#endif
