
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file interpolation.hpp
    \brief base class for 1-D interpolations
*/

#ifndef quantlib_interpolation_hpp
#define quantlib_interpolation_hpp

#include <ql/Patterns/bridge.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    //! abstract base class for interpolation implementations
    class InterpolationImpl {
      public:
        virtual ~InterpolationImpl() {}
        virtual double xMin() const = 0;
        virtual double xMax() const = 0;
        virtual bool isInRange(double) const = 0;
        virtual double value(double) const = 0;
        virtual double primitive(double) const = 0;
        virtual double derivative(double) const = 0;
        virtual double secondDerivative(double) const = 0;
    };

    //! base class for 1-D interpolations.
    /*! Classes derived from this class will provide interpolated
        values from two sequences of equal length, representing
        discretized values of a variable and a function of the former,
        respectively.
    */
    class Interpolation : public Bridge<Interpolation,InterpolationImpl> {
      public:
        typedef double argument_type;
        typedef double result_type;
      #if defined(QL_PATCH_MICROSOFT)
      public:
      #else
      protected:
      #endif
        //! basic template implementation
        template <class I1, class I2>
        class templateImpl : public InterpolationImpl {
          public:
            templateImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin) {
                QL_REQUIRE(xEnd_-xBegin_ >= 2,
                           "not enough points to interpolate");
                #if defined(QL_EXTRA_SAFETY_CHECKS)
                for (I1 i=xBegin_, j=xBegin_+1; j!=xEnd_; i++, j++)
                    QL_REQUIRE(*j > *i,
                               "Interpolation : unsorted x values");
                #endif
            }
            double xMin() const {
                return *xBegin_;
            }
            double xMax() const {
                return *(xEnd_-1);
            }
            bool isInRange(double x) const {
                return x >= xMin() && x <= xMax();
            }
          protected:
            Size locate(double x) const {
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
      public:
        Interpolation() {}
        double operator()(double x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->value(x);
        }
        double primitive(double x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->primitive(x);
        }
        double derivative(double x, bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->derivative(x);
        }
        double secondDerivative(double x, 
                                bool allowExtrapolation = false) const {
            checkRange(x,allowExtrapolation);
            return impl_->secondDerivative(x);
        }
        double xMin() const { return impl_->xMin(); }
        double xMax() const { return impl_->xMax(); }
        bool isInRange(double x) const { return impl_->isInRange(x); }
      protected:
        void checkRange(double x, bool allowExtrapolation) const {
            QL_REQUIRE(allowExtrapolation || impl_->isInRange(x),
                       "Interpolation::operator() : "
                       "\ninterpolation range is ["
                       + DoubleFormatter::toString(impl_->xMin()) +
                       ", "
                       + DoubleFormatter::toString(impl_->xMax()) +
                       "]: extrapolation at "
                       + DoubleFormatter::toString(x) +
                       " not allowed");
        }
    };

}


#endif
