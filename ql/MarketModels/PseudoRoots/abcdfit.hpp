/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sabrinterpolation.hpp
    \brief SABR interpolation interpolation between discrete points
*/

#ifndef quantlib_abcd_fit_hpp
#define quantlib_abcd_fit_hpp

#include <ql/Math/interpolation.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Utilities/null.hpp>
#include <vector>
#include <ql/MarketModels/PseudoRoots/abcd.hpp>

/*! \file abcdfit.hpp
    \brief Abcd fit between discrete points
*/

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class AbcdFitImpl;

        class AbcdCoefficientHolder {
          public:
            // Constructor
            AbcdCoefficientHolder(Real a, Real b, Real c, Real d,
                std::vector<Time> fixingTimes, bool aIsFixed = true, 
                bool dIsFixed = true) 
                : fixingTimes_(fixingTimes),
                  a_(a), b_(b), c_(c), d_(d),
                  aIsFixed_(aIsFixed), bIsFixed_(false),
                  cIsFixed_(false), dIsFixed_(dIsFixed),
                  error_(Null<Real>()), maxError_(Null<Real>()),
                  AbcdEndCriteria_(EndCriteria::none) {   }

            std::vector<Time> fixingTimes_;
            //! Abcd parameters
            Real a_, b_, c_, d_;
            bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
            Real error_, maxError_;
			EndCriteria::Type AbcdEndCriteria_;
        };
    }

    class AbcdFit : public Interpolation {
      public:
        template <class I1, class I2>
        AbcdFit( const I1& xBegin,   /* fixing times */
                 const I1& xEnd,     
                 const I2& yBegin,   /* variances*/
                 Real a, Real b, Real c, Real d,
                 std::vector<Time> fixingTimes,
                 const boost::shared_ptr<OptimizationMethod>& method
                       = boost::shared_ptr<OptimizationMethod>()) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(
                        new detail::AbcdFitImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                       a, b, c, d, fixingTimes,
                                                       method));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::AbcdCoefficientHolder>(
                                                                       impl_);
        }
        // Inspectors
        Real a() const { return coeffs_->a_; }
        Real b() const { return coeffs_->b_; }
        Real c() const { return coeffs_->c_; }
        Real d() const { return coeffs_->d_; }
        Real interpolationError() const { return coeffs_->error_; }
        Real interpolationMaxError() const { return coeffs_->maxError_; }     
		EndCriteria::Type endCriteria(){ return coeffs_->AbcdEndCriteria_; }

	private:
        boost::shared_ptr<detail::AbcdCoefficientHolder> coeffs_;
	};


    namespace detail {

        template <class I1, class I2>
        class AbcdFitImpl
            : public Interpolation::templateImpl<I1,I2>,
              public AbcdCoefficientHolder {
          private:
            // function to minimize
            class AbcdError;
            friend class AbcdError;
            class AbcdError : public CostFunction {
              public:
                AbcdError(AbcdFitImpl* abcd)
                : abcd_(abcd) {}
                Real value(const Array& x) const {
                    if (!abcd_->aIsFixed_) abcd_->a_ = x[0];
                    if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                    if (!abcd_->cIsFixed_) abcd_->c_ = x[2];
                    if (!abcd_->dIsFixed_) abcd_->d_ = x[3];
                    return abcd_->interpolationSquaredNonNormalizedError();
                }
              private:
                AbcdFitImpl* abcd_;
            };
            // optimization constraints
            class AbcdConstraint : public Constraint {
              private:
                class Impl : public Constraint::Impl {
                  public:
                    bool test(const Array& params) const {
                        return params[0] + params[3] > 0.0  // a + d
                            && params[2] > 0.0              // c
                            && params[3] > 0.0;             // d
                    }
                };
              public:
                AbcdConstraint()
                 : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl)) {}
            };
             // optimization method used for fitting
            boost::shared_ptr<OptimizationMethod> method_;

          public:  
            AbcdFitImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                        Real a, Real b, Real c, Real d, std::vector<Time> fixingTimes,
                        const boost::shared_ptr<OptimizationMethod>& method)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              AbcdCoefficientHolder(a, b, c, d, fixingTimes), method_(method)
            {
                calculate();
            }

            void calculate() {
                // there is nothing to optimize
                if (aIsFixed_ && bIsFixed_ && cIsFixed_ && dIsFixed_)
                {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError(); 
				    AbcdEndCriteria_ = EndCriteria::none;
                    return;
                } else {
                    AbcdConstraint constraint;
                    AbcdError costFunction(this);

                    if (!method_) {
                        boost::shared_ptr<LineSearch> lineSearch(
                            new ArmijoLineSearch(1e-12, 0.15, 0.55));
                        method_ = boost::shared_ptr<OptimizationMethod>(
                            new ConjugateGradient(lineSearch));
                        method_->setEndCriteria(EndCriteria(100000, 1e-12));
                        Array guess(4);
                        guess[0] =-0.0597;  // a
                        guess[1] = 0.1677;  // b
                        guess[2] = 0.5403;  // c
                        guess[3] = 0.1710;  // d
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
				    Array result = problem.minimumValue();
                    if (!aIsFixed_) a_ = result[0];
                    if (!bIsFixed_) b_ = result[1];
                    if (!cIsFixed_) c_ = result[2];
                    if (!dIsFixed_) d_ = result[3];

                    QL_ENSURE(d_>0.0, "d must be positive");
                    QL_ENSURE(a_+d_>0.0, "a+d must be positive");
                    QL_ENSURE(c_>0.0, "c must be positive");

				    AbcdEndCriteria_ = endCriteria();
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError(); 
                }
            }

            Real value(Real x) const {
                 boost::shared_ptr<Abcd>
                      instVol(new Abcd(a_,b_,c_,d_,x,x));
                return instVol->variance(x);
            }
            Real primitive(Real x) const {
                QL_FAIL("Abcd primitive not implemented");
            }
            Real derivative(Real x) const {
                QL_FAIL("Abcd derivative not implemented");
            }
            Real secondDerivative(Real x) const {
                QL_FAIL("Abcd secondDerivative not implemented");
            }

            Real interpolationSquaredNonNormalizedError() const {
                Real error, totalError = 0.0;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {
                    error = value(*i) - *j;
                    totalError += error*error;
                }
                return totalError;
            }

            Real interpolationError() const {
                Real normalizedError = interpolationSquaredNonNormalizedError()
                    /(this->xEnd_-this->xBegin_);
                return std::sqrt(normalizedError);
            }

            Real interpolationMaxError() const {
                Real error, maxError = QL_MIN_REAL;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {                    
                    error = std::fabs(value(*i) - *j);
                    maxError = std::max(maxError, error);
                }
                return maxError;
            }

			EndCriteria::Type endCriteria() {
				return method_->endCriteria().criteria(); 
			}
        };

    }
}

#endif
