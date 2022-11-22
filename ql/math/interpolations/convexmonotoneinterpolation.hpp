/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

/*! \file convexmonotoneinterpolation.hpp
    \brief convex monotone interpolation method
*/

#ifndef quantlib_convex_monotone_interpolation_hpp
#define quantlib_convex_monotone_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <map>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2> class ConvexMonotoneImpl;
        class SectionHelper;
    }

    //! Convex monotone yield-curve interpolation method.
    /*! Enhances implementation of the convex monotone method
        described in "Interpolation Methods for Curve Construction" by
        Hagan & West AMF Vol 13, No2 2006.

        A setting of monotonicity = 1 and quadraticity = 0 will
        reproduce the basic Hagan/West method. However, this can
        produce excessive gradients which can mean P&L swings for some
        curves.  Setting monotonicity < 1 and/or quadraticity > 0
        produces smoother curves.  Extra enhancement to avoid negative
        values (if required) is in place.

        \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    template <class I1, class I2>
    class ConvexMonotoneInterpolation : public Interpolation {
        typedef std::map<Real, ext::shared_ptr<detail::SectionHelper> >
                                                                   helper_map;
      public:
        ConvexMonotoneInterpolation(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin, Real quadraticity,
                                    Real monotonicity, bool forcePositive,
                                    bool flatFinalPeriod = false,
                                    const helper_map& preExistingHelpers =
                                                               helper_map()) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(
                   new detail::ConvexMonotoneImpl<I1,I2>(xBegin,
                                                         xEnd,
                                                         yBegin,
                                                         quadraticity,
                                                         monotonicity,
                                                         forcePositive,
                                                         flatFinalPeriod,
                                                         preExistingHelpers));
            impl_->update();
        }

        ConvexMonotoneInterpolation(Interpolation& interp)
        : Interpolation(interp) {}

        std::map<Real, ext::shared_ptr<detail::SectionHelper> >
        getExistingHelpers() {
            ext::shared_ptr<detail::ConvexMonotoneImpl<I1,I2> > derived =
                ext::dynamic_pointer_cast<detail::ConvexMonotoneImpl<I1,I2>,
                                            Interpolation::Impl>(impl_);
            return derived->getExistingHelpers();
        }
    };

    //! Convex-monotone interpolation factory and traits
    /*! \ingroup interpolations */
    class ConvexMonotone {
      public:
        static const bool global = true;
        static const Size requiredPoints = 2;
        static const Size dataSizeAdjustment = 1;

        explicit ConvexMonotone(Real quadraticity = 0.3,
                                Real monotonicity = 0.7,
                                bool forcePositive = true)
        : quadraticity_(quadraticity), monotonicity_(monotonicity),
          forcePositive_(forcePositive) {}

        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return ConvexMonotoneInterpolation<I1,I2>(xBegin, xEnd, yBegin,
                                                      quadraticity_,
                                                      monotonicity_,
                                                      forcePositive_,
                                                      false);
        }

        template <class I1, class I2>
        Interpolation localInterpolate(const I1& xBegin, const I1& xEnd,
                                       const I2& yBegin, Size localisation,
                                       Interpolation& prevInterpolation,
                                       Size finalSize) const {
            Size length = std::distance(xBegin, xEnd);
            if (length - localisation == 1) { // the first time this
                                              // function is called
                if (length == finalSize) {
                    return ConvexMonotoneInterpolation<I1,I2>(xBegin, xEnd,
                                                              yBegin,
                                                              quadraticity_,
                                                              monotonicity_,
                                                              forcePositive_,
                                                              false);
                } else {
                    return ConvexMonotoneInterpolation<I1,I2>(xBegin, xEnd,
                                                              yBegin,
                                                              quadraticity_,
                                                              monotonicity_,
                                                              forcePositive_,
                                                              true);
                }
            }

            ConvexMonotoneInterpolation<I1,I2> interp(prevInterpolation);
            if (length == finalSize) {
                return ConvexMonotoneInterpolation<I1,I2>(
                                                 xBegin, xEnd, yBegin,
                                                 quadraticity_,
                                                 monotonicity_,
                                                 forcePositive_,
                                                 false,
                                                 interp.getExistingHelpers());
            } else {
                return ConvexMonotoneInterpolation<I1,I2>(
                                                 xBegin, xEnd, yBegin,
                                                 quadraticity_,
                                                 monotonicity_,
                                                 forcePositive_,
                                                 true,
                                                 interp.getExistingHelpers());
            }
        }
      private:
        Real quadraticity_, monotonicity_;
        bool forcePositive_;
    };


    namespace detail {

        class SectionHelper {
          public:
            virtual ~SectionHelper() = default;
            virtual Real value(Real x) const = 0;
            virtual Real primitive(Real x) const = 0;
            virtual Real fNext() const = 0;
        };

        //the first value in the y-vector is ignored.
        template <class I1, class I2>
        class ConvexMonotoneImpl : public Interpolation::templateImpl<I1, I2> {
            typedef std::map<Real, ext::shared_ptr<SectionHelper> >
                                                                   helper_map;
          public:
            enum SectionType {
                EverywhereConstant,
                ConstantGradient,
                QuadraticMinimum,
                QuadraticMaximum
            };

            ConvexMonotoneImpl(const I1& xBegin,
                               const I1& xEnd,
                               const I2& yBegin,
                               Real quadraticity,
                               Real monotonicity,
                               bool forcePositive,
                               bool constantLastPeriod,
                               const helper_map& preExistingHelpers)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin,
                                                 ConvexMonotone::requiredPoints),
              preSectionHelpers_(preExistingHelpers),
              forcePositive_(forcePositive),
              constantLastPeriod_(constantLastPeriod),
              quadraticity_(quadraticity), monotonicity_(monotonicity),
              length_(xEnd-xBegin) {

                QL_REQUIRE(monotonicity_ >= 0 && monotonicity_ <= 1,
                           "Monotonicity must lie between 0 and 1");
                QL_REQUIRE(quadraticity_ >= 0 && quadraticity_ <= 1,
                           "Quadraticity must lie between 0 and 1");
                QL_REQUIRE(length_ >= 2,
                           "Single point provided, not supported by convex "
                           "monotone method as first point is ignored");
                QL_REQUIRE((length_ - preExistingHelpers.size()) > 1,
                            "Too many existing helpers have been supplied");
            }

            void update() override;

            Real value(Real x) const override;
            Real primitive(Real x) const override;
            Real derivative(Real) const override {
                QL_FAIL("Convex-monotone spline derivative not implemented");
            }
            Real secondDerivative(Real) const override {
                QL_FAIL("Convex-monotone spline second derivative "
                        "not implemented");
            }

            helper_map getExistingHelpers() {
                helper_map retArray(sectionHelpers_);
                if (constantLastPeriod_)
                    retArray.erase(*(this->xEnd_-1));
                return retArray;
            }
          private:
            helper_map sectionHelpers_;
            helper_map preSectionHelpers_;
            ext::shared_ptr<SectionHelper> extrapolationHelper_;
            bool forcePositive_, constantLastPeriod_;
            Real quadraticity_;
            Real monotonicity_;
            Size length_;
        };


        class ComboHelper : public SectionHelper {
          public:
            ComboHelper(ext::shared_ptr<SectionHelper>& quadraticHelper,
                        ext::shared_ptr<SectionHelper>& convMonoHelper,
                        Real quadraticity)
            : quadraticity_(quadraticity),
              quadraticHelper_(quadraticHelper),
              convMonoHelper_(convMonoHelper) {
                QL_REQUIRE(quadraticity < 1.0 && quadraticity > 0.0,
                           "Quadratic value must lie between 0 and 1"); }

            Real value(Real x) const override {
                return( quadraticity_*quadraticHelper_->value(x) + (1.0-quadraticity_)*convMonoHelper_->value(x) );
            }
            Real primitive(Real x) const override {
                return( quadraticity_*quadraticHelper_->primitive(x) + (1.0-quadraticity_)*convMonoHelper_->primitive(x) );
            }
            Real fNext() const override {
                return( quadraticity_*quadraticHelper_->fNext() + (1.0-quadraticity_)*convMonoHelper_->fNext() );
            }

          private:
            Real quadraticity_;
            ext::shared_ptr<SectionHelper> quadraticHelper_;
            ext::shared_ptr<SectionHelper> convMonoHelper_;
        };

        class EverywhereConstantHelper : public SectionHelper {
          public:
            EverywhereConstantHelper(Real value, Real prevPrimitive, Real xPrev)
            : value_(value), prevPrimitive_(prevPrimitive), xPrev_(xPrev)
            {}

            Real value(Real) const override { return value_; }
            Real primitive(Real x) const override { return prevPrimitive_ + (x - xPrev_) * value_; }
            Real fNext() const override { return value_; }

          private:
            Real value_;
            Real prevPrimitive_;
            Real xPrev_;
        };

        class ConvexMonotone2Helper : public SectionHelper {
          public:
            ConvexMonotone2Helper(Real xPrev, Real xNext,
                                  Real gPrev, Real gNext,
                                  Real fAverage, Real eta2,
                                  Real prevPrimitive)
            : xPrev_(xPrev), xScaling_(xNext-xPrev), gPrev_(gPrev),
              gNext_(gNext), fAverage_(fAverage), eta2_(eta2),
              prevPrimitive_(prevPrimitive)
            {}

            Real value(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                if (xVal <= eta2_) {
                    return( fAverage_ + gPrev_ );
                } else {
                    return( fAverage_ + gPrev_ + (gNext_-gPrev_)/((1-eta2_)*(1-eta2_))*(xVal-eta2_)*(xVal-eta2_) );
                }
            }

            Real primitive(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                if (xVal <= eta2_) {
                    return( prevPrimitive_ + xScaling_*(fAverage_*xVal + gPrev_*xVal) );
                } else {
                    return( prevPrimitive_ + xScaling_*(fAverage_*xVal + gPrev_*xVal + (gNext_-gPrev_)/((1-eta2_)*(1-eta2_)) *
                            (1.0/3.0*(xVal*xVal*xVal - eta2_*eta2_*eta2_) - eta2_*xVal*xVal + eta2_*eta2_*xVal) ) );
                }
            }
            Real fNext() const override { return (fAverage_ + gNext_); }

          private:
            Real xPrev_, xScaling_, gPrev_, gNext_, fAverage_, eta2_, prevPrimitive_;
        };

        class ConvexMonotone3Helper : public SectionHelper {
          public:
            ConvexMonotone3Helper(Real xPrev, Real xNext,
                                  Real gPrev, Real gNext,
                                  Real fAverage, Real eta3,
                                  Real prevPrimitive)
              : xPrev_(xPrev), xScaling_(xNext-xPrev), gPrev_(gPrev),
                gNext_(gNext), fAverage_(fAverage), eta3_(eta3), prevPrimitive_(prevPrimitive)
            {}

            Real value(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                if (xVal <= eta3_) {
                    return( fAverage_ + gNext_ + (gPrev_-gNext_) / (eta3_*eta3_) * (eta3_-xVal)*(eta3_-xVal) );
                } else {
                    return( fAverage_ + gNext_ );
                }
            }

            Real primitive(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                if (xVal <= eta3_) {
                    return( prevPrimitive_ + xScaling_ * (fAverage_*xVal + gNext_*xVal + (gPrev_-gNext_)/(eta3_*eta3_) *
                            (1.0/3.0 * xVal*xVal*xVal - eta3_*xVal*xVal + eta3_*eta3_*xVal) ) );
                } else {
                    return( prevPrimitive_ + xScaling_ * (fAverage_*xVal + gNext_*xVal + (gPrev_-gNext_)/(eta3_*eta3_) *
                            (1.0/3.0 * eta3_*eta3_*eta3_)) );
                }
            }
            Real fNext() const override { return (fAverage_ + gNext_); }

          private:
            Real xPrev_, xScaling_, gPrev_, gNext_, fAverage_, eta3_, prevPrimitive_;
        };

        class ConvexMonotone4Helper : public SectionHelper {
          public:
            ConvexMonotone4Helper(Real xPrev,  Real xNext,
                                  Real gPrev, Real gNext,
                                  Real fAverage, Real eta4,
                                  Real prevPrimitive)
            : xPrev_(xPrev), xScaling_(xNext-xPrev), gPrev_(gPrev),
              gNext_(gNext), fAverage_(fAverage), eta4_(eta4), prevPrimitive_(prevPrimitive) {
                A_ = -0.5*(eta4_*gPrev_ + (1-eta4_)*gNext_);
            }

            Real value(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                if (xVal <= eta4_) {
                    return(fAverage_ + A_ + (gPrev_-A_)*(eta4_-xVal)*(eta4_-xVal)/(eta4_*eta4_) );
                } else {
                    return(fAverage_ + A_ + (gNext_-A_)*(xVal-eta4_)*(xVal-eta4_)/((1-eta4_)*(1-eta4_)) );
                }
            }

            Real primitive(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                Real retVal;
                if (xVal <= eta4_) {
                    retVal = prevPrimitive_ + xScaling_ * (fAverage_ + A_ + (gPrev_-A_)/(eta4_*eta4_) *
                            (eta4_*eta4_ - eta4_*xVal + 1.0/3.0*xVal*xVal)) * xVal;
                } else {
                    retVal = prevPrimitive_ + xScaling_ *(fAverage_*xVal + A_*xVal + (gPrev_-A_)*(1.0/3.0*eta4_) +
                             (gNext_-A_)/((1-eta4_)*(1-eta4_)) *
                             (1.0/3.0*xVal*xVal*xVal - eta4_*xVal*xVal + eta4_*eta4_*xVal - 1.0/3.0*eta4_*eta4_*eta4_));
                }
                return retVal;
            }
            Real fNext() const override { return (fAverage_ + gNext_); }

          protected:
            Real xPrev_, xScaling_, gPrev_, gNext_, fAverage_, eta4_, prevPrimitive_;
            Real A_;
        };

        class ConvexMonotone4MinHelper : public ConvexMonotone4Helper {
          public:
            ConvexMonotone4MinHelper(Real xPrev,
                                     Real xNext,
                                     Real gPrev,
                                     Real gNext,
                                     Real fAverage,
                                     Real eta4,
                                     Real prevPrimitive)
            : ConvexMonotone4Helper(xPrev, xNext, gPrev, gNext, fAverage, eta4, prevPrimitive) {
                if ( A_+ fAverage_ <= 0.0 ) {
                    splitRegion_ = true;
                    Real fPrev = gPrev_+fAverage_;
                    Real fNext = gNext_+fAverage_;
                    Real reqdShift = (eta4_*fPrev + (1-eta4_)*fNext)/3.0 - fAverage_;
                    Real reqdPeriod = reqdShift * xScaling_ / (fAverage_+reqdShift);
                    Real xAdjust = xScaling_ - reqdPeriod;
                    xRatio_ =  xAdjust/xScaling_;

                    fAverage_ += reqdShift;
                    gNext_ = fNext - fAverage_;
                    gPrev_ = fPrev - fAverage_;
                    A_ = -(eta4_ * gPrev_ + (1.0-eta4)*gNext_)/2.0;
                    x2_ = xPrev_ + xAdjust  * eta4_;
                    x3_ = xPrev_ + xScaling_ - xAdjust*(1.0-eta4_);
                }
            }

            Real value(Real x) const override {
                if (!splitRegion_)
                    return ConvexMonotone4Helper::value(x);

                Real xVal = (x-xPrev_)/xScaling_;
                if (x <= x2_) {
                    xVal /= xRatio_;
                    return(fAverage_ + A_ + (gPrev_-A_)*(eta4_-xVal)*(eta4_-xVal)/(eta4_*eta4_));
                } else if (x < x3_) {
                    return 0.0;
                } else {
                    xVal = 1.0 - (1.0 - xVal) / xRatio_;
                    return(fAverage_ + A_ + (gNext_-A_)*(xVal-eta4_)*(xVal-eta4_)/((1-eta4_)*(1-eta4_)) );
                }
            }

            Real primitive(Real x) const override {
                if (!splitRegion_)
                    return ConvexMonotone4Helper::primitive(x);

                Real xVal = (x-xPrev_)/xScaling_;
                if (x <= x2_) {
                    xVal /= xRatio_;
                    return( prevPrimitive_ + xScaling_*xRatio_*(fAverage_ + A_ + (gPrev_-A_)/(eta4_*eta4_) *
                            (eta4_*eta4_ - eta4_*xVal + 1.0/3.0*xVal*xVal)) * xVal );
                } else if (x <= x3_) {
                    return( prevPrimitive_ + xScaling_*xRatio_*(fAverage_*eta4_ + A_*eta4_ + (gPrev_-A_)/(eta4_*eta4_) *
                            (1.0/3.0*eta4_*eta4_*eta4_)) );
                } else {
                    xVal = 1.0 - (1.0-xVal)/xRatio_;
                    return( prevPrimitive_ + xScaling_*xRatio_*(fAverage_*xVal + A_*xVal + (gPrev_-A_)*(1.0/3.0*eta4_) +
                            (gNext_-A_) / ((1.0-eta4_)*(1.0-eta4_)) *
                            (1.0/3.0*xVal*xVal*xVal - eta4_*xVal*xVal + eta4_*eta4_*xVal - 1.0/3.0*eta4_*eta4_*eta4_)) );
                }
            }

          private:
            bool splitRegion_ = false;
            Real xRatio_, x2_, x3_;
        };

        class ConstantGradHelper : public SectionHelper {
          public:
            ConstantGradHelper(Real fPrev, Real prevPrimitive,
                               Real xPrev, Real xNext, Real fNext)
            : fPrev_(fPrev), prevPrimitive_(prevPrimitive),
            xPrev_(xPrev), fGrad_((fNext-fPrev)/(xNext-xPrev)),fNext_(fNext)
            {}

            Real value(Real x) const override { return (fPrev_ + (x - xPrev_) * fGrad_); }
            Real primitive(Real x) const override {
                return (prevPrimitive_+(x-xPrev_)*(fPrev_+0.5*(x-xPrev_)*fGrad_));
            }
            Real fNext() const override { return fNext_; }

          private:
            Real fPrev_, prevPrimitive_, xPrev_, fGrad_, fNext_;
        };

        class QuadraticHelper : public SectionHelper {
          public:
            QuadraticHelper(Real xPrev, Real xNext,
                               Real fPrev, Real fNext,
                               Real fAverage,
                               Real prevPrimitive)
            : xPrev_(xPrev), xNext_(xNext), fPrev_(fPrev),
              fNext_(fNext), fAverage_(fAverage),
              prevPrimitive_(prevPrimitive) {
                a_ = 3*fPrev_ + 3*fNext_ - 6*fAverage_;
                b_ = -(4*fPrev_ + 2*fNext_ - 6*fAverage_);
                c_ = fPrev_;
                xScaling_ = xNext_-xPrev_;
            }

            Real value(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                return( a_*xVal*xVal + b_*xVal + c_ );
            }

            Real primitive(Real x) const override {
                Real xVal = (x-xPrev_)/xScaling_;
                return( prevPrimitive_ + xScaling_ * (a_/3*xVal*xVal + b_/2*xVal + c_) * xVal );
            }

            Real fNext() const override { return fNext_; }

          private:
            Real xPrev_, xNext_, fPrev_, fNext_, fAverage_, prevPrimitive_;
            Real xScaling_, a_, b_, c_;
        };

        class QuadraticMinHelper : public SectionHelper {
          public:
            QuadraticMinHelper(
                Real xPrev, Real xNext, Real fPrev, Real fNext, Real fAverage, Real prevPrimitive)
            : x1_(xPrev), x4_(xNext), primitive1_(prevPrimitive), fAverage_(fAverage),
              fPrev_(fPrev), fNext_(fNext) {
                a_ = 3*fPrev_ + 3*fNext_ - 6*fAverage_;
                b_ = -(4*fPrev_ + 2*fNext_ - 6*fAverage_);
                c_ = fPrev_;
                Real d = b_*b_-4*a_*c_;
                xScaling_ = x4_-x1_;
                if (d > 0) {
                    Real aAv = 36;
                    Real bAv = -24*(fPrev_+fNext_);
                    Real cAv = 4*(fPrev_*fPrev_ + fPrev_*fNext_ + fNext_*fNext_);
                    Real dAv = bAv*bAv - 4.0*aAv*cAv;
                    if (dAv >= 0.0) {
                        splitRegion_ = true;
                        Real avRoot = (-bAv - std::sqrt(dAv))/(2*aAv);

                        xRatio_ = fAverage_ / avRoot;
                        xScaling_ *= xRatio_;

                        a_ = 3*fPrev_ + 3*fNext_ - 6*avRoot;
                        b_ = -(4*fPrev_ + 2*fNext_ - 6*avRoot);
                        c_ = fPrev_;
                        Real xRoot = -b_/(2*a_);
                        x2_ = x1_ + xRatio_ * (x4_-x1_) * xRoot;
                        x3_ = x4_ - xRatio_ * (x4_-x1_) * (1-xRoot);
                        primitive2_ =
                            primitive1_ + xScaling_*(a_/3*xRoot*xRoot + b_/2*xRoot + c_)*xRoot;
                    }
                }
            }

            Real value(Real x) const override {
                Real xVal = (x - x1_) / (x4_-x1_);
                if (splitRegion_) {
                    if (x <= x2_) {
                        xVal /= xRatio_;
                    } else if (x < x3_) {
                        return 0.0;
                    } else {
                        xVal = 1.0 - (1.0 - xVal) / xRatio_;
                    }
                }

                return c_ + b_*xVal + a_*xVal*xVal;
            }

            Real primitive(Real x) const override {
                Real xVal = (x - x1_) / (x4_-x1_);
                if (splitRegion_) {
                    if (x < x2_) {
                        xVal /= xRatio_;
                    } else if (x < x3_) {
                        return primitive2_;
                    } else {
                        xVal = 1.0 - (1.0 - xVal) / xRatio_;
                    }
                }
                return primitive1_ + xScaling_ * (a_/3*xVal*xVal+ b_/2*xVal+c_)*xVal;
            }

            Real fNext() const override { return fNext_; }

          private:
            bool splitRegion_ = false;
            Real x1_, x2_, x3_, x4_;
            Real a_, b_, c_;
            Real primitive1_, primitive2_;
            Real fAverage_, fPrev_, fNext_, xScaling_, xRatio_ = 1.0;
        };

        template <class I1, class I2>
        void ConvexMonotoneImpl<I1,I2>::update() {
            sectionHelpers_.clear();
            if (length_ == 2) { //single period
                ext::shared_ptr<SectionHelper> singleHelper(
                              new EverywhereConstantHelper(this->yBegin_[1],
                                                           0.0,
                                                           this->xBegin_[0]));
                sectionHelpers_[this->xBegin_[1]] = singleHelper;
                extrapolationHelper_ = singleHelper;
                return;
            }

            std::vector<Real> f(length_);
            sectionHelpers_ = preSectionHelpers_;
            Size startPoint = sectionHelpers_.size()+1;

            //first derive the boundary forwards.
            for (Size i=startPoint; i<length_-1; ++i) {
                Real dxPrev = this->xBegin_[i] - this->xBegin_[i-1];
                Real dx = this->xBegin_[i+1] - this->xBegin_[i];
                f[i] = dx/(dx+dxPrev) * this->yBegin_[i]
                     + dxPrev/(dx+dxPrev) * this->yBegin_[i+1];
            }

            if (startPoint > 1)
                f[startPoint-1] = preSectionHelpers_.rbegin()->second->fNext();
            if (startPoint == 1)
                f[0] = 1.5 * this->yBegin_[1] - 0.5 * f[1];

            f[length_-1] = 1.5 * this->yBegin_[length_-1] - 0.5 * f[length_-2];

            if (forcePositive_) {
                if (f[0] < 0)
                    f[0] = 0.0;
                if (f[length_-1] < 0.0)
                    f[length_-1] = 0.0;
            }

            Real primitive = 0.0;
            for (Size i = 0; i < startPoint-1; ++i)
                primitive +=
                    this->yBegin_[i+1] * (this->xBegin_[i+1]-this->xBegin_[i]);

            Size endPoint = length_;
            //constantLastPeriod_ = false;
            if (constantLastPeriod_)
                endPoint = endPoint-1;

            for (Size i=startPoint; i< endPoint; ++i) {
                Real gPrev = f[i-1] - this->yBegin_[i];
                Real gNext = f[i] - this->yBegin_[i];
                //first deal with the zero gradient case
                if ( std::fabs(gPrev) < 1.0E-14
                     && std::fabs(gNext) < 1.0E-14 ) {
                    ext::shared_ptr<SectionHelper> singleHelper(
                                     new ConstantGradHelper(f[i-1], primitive,
                                                            this->xBegin_[i-1],
                                                            this->xBegin_[i],
                                                            f[i]));
                    sectionHelpers_[this->xBegin_[i]] = singleHelper;
                } else {
                    Real quadraticity = quadraticity_;
                    ext::shared_ptr<SectionHelper> quadraticHelper;
                    ext::shared_ptr<SectionHelper> convMonotoneHelper;
                    if (quadraticity_ > 0.0) {
                        if (gPrev >= -2.0*gNext && gPrev > -0.5*gNext && forcePositive_) {
                            quadraticHelper =
                                ext::shared_ptr<SectionHelper>(
                                    new QuadraticMinHelper(this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           f[i-1], f[i],
                                                           this->yBegin_[i],
                                                           primitive) );
                        } else {
                            quadraticHelper =
                                ext::shared_ptr<SectionHelper>(
                                    new QuadraticHelper(this->xBegin_[i-1],
                                                        this->xBegin_[i],
                                                        f[i-1], f[i],
                                                        this->yBegin_[i],
                                                        primitive) );
                        }
                    }
                    if (quadraticity_ < 1.0) {

                        if ((gPrev > 0.0 && -0.5*gPrev >= gNext && gNext >= -2.0*gPrev) ||
                            (gPrev < 0.0 && -0.5*gPrev <= gNext && gNext <= -2.0*gPrev)) {
                            quadraticity = 1.0;
                            if (quadraticity_ == 0) {
                                if (forcePositive_) {
                                    quadraticHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new QuadraticMinHelper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           f[i-1], f[i],
                                                           this->yBegin_[i],
                                                           primitive) );
                                } else {
                                    quadraticHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new QuadraticHelper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           f[i-1], f[i],
                                                           this->yBegin_[i],
                                                           primitive) );
                                }
                            }
                        }
                        else if ( (gPrev < 0.0 && gNext > -2.0*gPrev) ||
                                  (gPrev > 0.0 && gNext < -2.0*gPrev)) {

                            Real eta = (gNext + 2.0*gPrev)/(gNext - gPrev);
                            Real b2 = (1.0 + monotonicity_)/2.0;
                            if (eta < b2) {
                                convMonotoneHelper =
                                    ext::shared_ptr<SectionHelper>(
                                        new ConvexMonotone2Helper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           eta, primitive));
                            } else {
                                if (forcePositive_) {
                                    convMonotoneHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new ConvexMonotone4MinHelper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           b2, primitive));
                                } else {
                                    convMonotoneHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new ConvexMonotone4Helper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           b2, primitive));
                                }
                            }
                        }
                        else if ( (gPrev > 0.0 && gNext < 0.0 && gNext > -0.5*gPrev) ||
                                  (gPrev < 0.0 && gNext > 0.0 && gNext < -0.5*gPrev) ) {
                            Real eta = gNext/(gNext-gPrev) * 3.0;
                            Real b3 = (1.0 - monotonicity_)/2.0;
                            if (eta > b3) {
                                convMonotoneHelper =
                                    ext::shared_ptr<SectionHelper>(
                                        new ConvexMonotone3Helper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           eta, primitive));
                            } else {
                                if (forcePositive_) {
                                    convMonotoneHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new ConvexMonotone4MinHelper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           b3, primitive));
                                } else {
                                    convMonotoneHelper =
                                        ext::shared_ptr<SectionHelper>(
                                            new ConvexMonotone4Helper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           b3, primitive));
                                }
                            }
                        } else {
                            Real eta = gNext/(gPrev + gNext);
                            Real b2 = (1.0 + monotonicity_)/2.0;
                            Real b3 = (1.0 - monotonicity_)/2.0;
                            if (eta > b2)
                                eta = b2;
                            if (eta < b3)
                                eta = b3;
                            if (forcePositive_) {
                                convMonotoneHelper =
                                    ext::shared_ptr<SectionHelper>(
                                        new ConvexMonotone4MinHelper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           eta, primitive));
                            } else {
                                convMonotoneHelper =
                                    ext::shared_ptr<SectionHelper>(
                                        new ConvexMonotone4Helper(
                                                           this->xBegin_[i-1],
                                                           this->xBegin_[i],
                                                           gPrev, gNext,
                                                           this->yBegin_[i],
                                                           eta, primitive));
                            }
                        }
                    }

                    if (quadraticity == 1.0) {
                        sectionHelpers_[this->xBegin_[i]] = quadraticHelper;
                    } else if (quadraticity == 0.0) {
                        sectionHelpers_[this->xBegin_[i]] = convMonotoneHelper;
                    } else {
                        sectionHelpers_[this->xBegin_[i]] =
                            ext::shared_ptr<SectionHelper>(
                                           new ComboHelper(quadraticHelper,
                                                           convMonotoneHelper,
                                                           quadraticity));
                    }

                }
                primitive +=
                    this->yBegin_[i] * (this->xBegin_[i]-this->xBegin_[i-1]);
            }

            if (constantLastPeriod_) {
                sectionHelpers_[this->xBegin_[length_-1]] =
                    ext::shared_ptr<SectionHelper>(
                        new EverywhereConstantHelper(this->yBegin_[length_-1],
                                                     primitive,
                                                     this->xBegin_[length_-2]));
                extrapolationHelper_ = sectionHelpers_[this->xBegin_[length_-1]];
            } else {
                extrapolationHelper_ =
                    ext::shared_ptr<SectionHelper>(
                        new EverywhereConstantHelper(
                                (sectionHelpers_.rbegin())->second->value(*(this->xEnd_-1)),
                                primitive,
                                *(this->xEnd_-1)));
            }
        }

        template <class I1, class I2>
        Real ConvexMonotoneImpl<I1,I2>::value(Real x) const {
            if (x >= *(this->xEnd_-1)) {
                return extrapolationHelper_->value(x);
            }

            return sectionHelpers_.upper_bound(x)->second->value(x);
        }

        template <class I1, class I2>
        Real ConvexMonotoneImpl<I1,I2>::primitive(Real x) const {
            if (x >= *(this->xEnd_-1)) {
                return extrapolationHelper_->primitive(x);
            }

            return sectionHelpers_.upper_bound(x)->second->primitive(x);
        }

    }

}

#endif
