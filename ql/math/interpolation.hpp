/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file interpolation.hpp
    \brief base class for 1-D interpolations
*/

#ifndef quantlib_interpolation_hpp
#define quantlib_interpolation_hpp

#include <ql/math/interpolations/extrapolation.hpp>
#include <ql/math/comparison.hpp>
#include <ql/errors.hpp>
#include <vector>
#include <algorithm>

namespace QuantLib {

    //! base class for 1-D interpolations.
    /*! Classes derived from this class will provide interpolated
        values from two sequences of equal length, representing
        discretized values of a variable and a function of the former,
        respectively.

        \warning Interpolations don't copy their underlying data;
                 instead, they store iterators through which they
                 access them.  This allow them to see changes in the
                 underlying data without having to propagate them
                 manually, but adds the requirement that the lifetime
                 of the underlying data exceeds or equals the lifetime
                 of the interpolation. It is up to the user to ensure
                 this: usually, a class will store as data members
                 both the data and the interpolation (see, e.g., the
                 InterpolatedCurve class) and call the update() method
                 on the latter when the data change.
    */
    class Interpolation : public Extrapolator {
      protected:
        //! abstract base class for interpolation implementations
        class Impl {
          public:
            virtual ~Impl() = default;
            virtual void update() = 0;
            virtual Real xMin() const = 0;
            virtual Real xMax() const = 0;
            virtual std::vector<Real> xValues() const = 0;
            virtual std::vector<Real> yValues() const = 0;
            virtual bool isInRange(Real) const = 0;
            virtual Real value(Real) const = 0;
            virtual Real primitive(Real) const = 0;
            virtual Real derivative(Real) const = 0;
            virtual Real secondDerivative(Real) const = 0;
        };
        ext::shared_ptr<Impl> impl_;
      public:
        typedef Real argument_type;
        typedef Real result_type;
        //! basic template implementation
        template <class I1, class I2>
        class templateImpl : public Impl {
          public:
            templateImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                         const int requiredPoints = 2)
            : xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin) {
                QL_REQUIRE(static_cast<int>(xEnd_-xBegin_) >= requiredPoints,
                           "not enough points to interpolate: at least " <<
                           requiredPoints <<
                           " required, " << static_cast<int>(xEnd_-xBegin_)<< " provided");
            }
            Real xMin() const override { return *xBegin_; }
            Real xMax() const override { return *(xEnd_ - 1); }
            std::vector<Real> xValues() const override { return std::vector<Real>(xBegin_, xEnd_); }
            std::vector<Real> yValues() const override {
                return std::vector<Real>(yBegin_,yBegin_+(xEnd_-xBegin_));
            }
            bool isInRange(Real x) const override {
#if defined(QL_EXTRA_SAFETY_CHECKS)
                for (I1 i=xBegin_, j=xBegin_+1; j!=xEnd_; ++i, ++j)
                    QL_REQUIRE(*j > *i, "unsorted x values");
                #endif
                Real x1 = xMin(), x2 = xMax();
                return (x >= x1 && x <= x2) || close(x,x1) || close(x,x2);
            }

          protected:
            Size locate(Real x) const {
                #if defined(QL_EXTRA_SAFETY_CHECKS)
                for (I1 i=xBegin_, j=xBegin_+1; j!=xEnd_; ++i, ++j)
                    QL_REQUIRE(*j > *i, "unsorted x values");
                #endif
                if (x < *xBegin_)
                    return 0;
                else if (x > *(xEnd_-1))
                    return xEnd_-xBegin_-2;
                else
                    return std::upper_bound(xBegin_,xEnd_-1,x)-xBegin_-1;
            }
            I1 xBegin_, xEnd_;
            I2 yBegin_;
        };

        Interpolation() = default;
        ~Interpolation() override = default;
        bool empty() const { return !impl_; }
        Real operator()(Real x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->value(x);
        }
        Real primitive(Real x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->primitive(x);
        }
        Real derivative(Real x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->derivative(x);
        }
        Real secondDerivative(Real x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->secondDerivative(x);
        }
        Real xMin() const {
            return impl_->xMin();
        }
        Real xMax() const {
            return impl_->xMax();
        }
        bool isInRange(Real x) const {
            return impl_->isInRange(x);
        }
        void update() {
            impl_->update();
        }
      protected:
        void checkRange(Real x, bool extrapolate) const {
            QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                       impl_->isInRange(x),
                       "interpolation range is ["
                       << impl_->xMin() << ", " << impl_->xMax()
                       << "]: extrapolation at " << x << " not allowed");
        }
    };

}

#endif


#ifndef id_f4be1d9a79c7464671fb61083c4d88d0
#define id_f4be1d9a79c7464671fb61083c4d88d0
inline bool test_f4be1d9a79c7464671fb61083c4d88d0(int* i) { return i != 0; }
#endif
