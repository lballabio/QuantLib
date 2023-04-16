/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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

/*! \file vannavolgainterpolation.hpp
    \brief Vanna/Volga interpolation between discrete points
*/

#ifndef quantlib_vanna_volga_interpolation_hpp
#define quantlib_vanna_volga_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2> class VannaVolgaInterpolationImpl;
    }

    //! %Vanna Volga interpolation between discrete points
    class VannaVolgaInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        VannaVolgaInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin,
                            Real spot,
                            DiscountFactor dDiscount,
                            DiscountFactor fDiscount,
                            Time T) {
            impl_ = std::make_shared<
                detail::VannaVolgaInterpolationImpl<I1,I2> >(
                    xBegin, xEnd, yBegin,
                    spot, dDiscount, fDiscount, T);
            impl_->update();
        }
    };

    //! %VannaVolga-interpolation factory and traits
    class VannaVolga {
      public:
        VannaVolga(Real spot,
                   DiscountFactor dDiscount,
                   DiscountFactor fDiscount,
                   Time T)
        :spot_(spot), dDiscount_(dDiscount), fDiscount_(fDiscount), T_(T)
        {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return VannaVolgaInterpolation(xBegin, xEnd, yBegin, spot_, dDiscount_, fDiscount_, T_);
        }
        static const Size requiredPoints = 3;
      private:
        Real spot_;
        DiscountFactor dDiscount_;
        DiscountFactor fDiscount_;
        Time T_;
    };

    namespace detail {

        template <class I1, class I2>
        class VannaVolgaInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            VannaVolgaInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin,
                                    Real spot,
                                    DiscountFactor dDiscount,
                                    DiscountFactor fDiscount,
                                    Time T)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                 VannaVolga::requiredPoints),
              spot_(spot), dDiscount_(dDiscount), fDiscount_(fDiscount), T_(T) {
                QL_REQUIRE(this->xEnd_-this->xBegin_ == 3,
                    "Vanna Volga Interpolator only interpolates 3 volatilities in strike space");
            }
            void update() override {
                //atmVol should be the second vol
                atmVol_ = this->yBegin_[1];
                fwd_ = spot_*fDiscount_/dDiscount_;
                for(Size i = 0; i < 3; i++){
                    premiaBS.push_back(blackFormula(Option::Call, this->xBegin_[i], fwd_, atmVol_ * std::sqrt(T_), dDiscount_));
                    premiaMKT.push_back(blackFormula(Option::Call, this->xBegin_[i], fwd_, this->yBegin_[i] * std::sqrt(T_), dDiscount_));
                    vegas.push_back(vega(this->xBegin_[i]));
                }
            }
            Real value(Real k) const override {
                Real x1 = vega(k)/vegas[0]
                    * (std::log(this->xBegin_[1]/k) * std::log(this->xBegin_[2]/k))
                    / (std::log(this->xBegin_[1]/this->xBegin_[0]) * std::log(this->xBegin_[2]/this->xBegin_[0]));
                Real x2 = vega(k)/vegas[1]
                    * (std::log(k/this->xBegin_[0]) * std::log(this->xBegin_[2]/k))
                    / (std::log(this->xBegin_[1]/this->xBegin_[0]) * std::log(this->xBegin_[2]/this->xBegin_[1]));
                Real x3 = vega(k)/vegas[2]
                    * (std::log(k/this->xBegin_[0]) * std::log(k/this->xBegin_[1]))
                    / (std::log(this->xBegin_[2]/this->xBegin_[0]) * std::log(this->xBegin_[2]/this->xBegin_[1]));

                Real cBS = blackFormula(Option::Call, k, fwd_, atmVol_ * std::sqrt(T_), dDiscount_);
                Real c = cBS + x1*(premiaMKT[0] - premiaBS[0]) + x2*(premiaMKT[1] - premiaBS[1]) + x3*(premiaMKT[2] - premiaBS[2]);
                Real std = blackFormulaImpliedStdDev(Option::Call, k, fwd_, c, dDiscount_);
                return std / sqrt(T_);
            }
            Real primitive(Real) const override {
                QL_FAIL("Vanna Volga primitive not implemented");
            }
            Real derivative(Real) const override {
                QL_FAIL("Vanna Volga derivative not implemented");
            }
            Real secondDerivative(Real) const override {
                QL_FAIL("Vanna Volga secondDerivative not implemented");
            }

          private:
            std::vector<Real> premiaBS;
            std::vector<Real> premiaMKT;
            std::vector<Real> vegas;
            Real atmVol_;
            Real spot_;
            Real fwd_;
            DiscountFactor dDiscount_;
            DiscountFactor fDiscount_;
            Time T_;

            Real vega(Real k) const {
                Real d1 = (std::log(fwd_/k) + 0.5 * std::pow(atmVol_, 2.0) * T_)/(atmVol_ * std::sqrt(T_));
                NormalDistribution norm;
                return spot_ * dDiscount_ * std::sqrt(T_) * norm(d1);
            }
        };

    }

}

#endif
