
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file interpolation2D.hpp
    \brief abstract base classes for 2-D interpolations
*/

#ifndef quantlib_interpolation2D_hpp
#define quantlib_interpolation2D_hpp

#include <ql/Patterns/bridge.hpp>
#include <ql/basicdataformatters.hpp>

namespace QuantLib {

    //! abstract base class for 2-D interpolation implementations
    class Interpolation2DImpl {
      public:
        virtual ~Interpolation2DImpl() {}
        virtual Real xMin() const = 0;
        virtual Real xMax() const = 0;
        virtual Real yMin() const = 0;
        virtual Real yMax() const = 0;
        virtual bool isInRange(Real x, Real y) const = 0;
        virtual Real value(Real x, Real y) const = 0;
    };

    //! base class for 2-D interpolations.
    /*! Classes derived from this class will provide interpolated
        values from two sequences of length \f$ N \f$ and \f$ M \f$,
        representing the discretized values of the \f$ x \f$ and \f$ y
        \f$ variables, and a \f$ N \times M \f$ matrix representing
        the tabulated function values.
    */
    class Interpolation2D
        : public Bridge<Interpolation2D,Interpolation2DImpl> {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;
        //! basic template implementation
        template <class I1, class I2, class M>
        class templateImpl : public Interpolation2DImpl {
          public:
            templateImpl(const I1& xBegin, const I1& xEnd,
                         const I2& yBegin, const I2& yEnd,
                         const M& zData)
            : xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin), yEnd_(yEnd),
              zData_(zData) {
                QL_REQUIRE(xEnd_-xBegin_ >= 2 && yEnd_-yBegin_ >= 2,
                           "not enough points to interpolate");
                #if defined(QL_EXTRA_SAFETY_CHECKS)
                for (I1 i=xBegin_, j=xBegin_+1; j!=xEnd_; i++, j++)
                    QL_REQUIRE(*j > *i, "unsorted x values");
                for (I2 k=yBegin_, l=yBegin_+1; l!=yEnd_; k++, l++)
                    QL_REQUIRE(*l > *k, "unsorted y values");
                #endif
            }
            Real xMin() const {
                return *xBegin_;
            }
            Real xMax() const {
                return *(xEnd_-1);
            }
            Real yMin() const {
                return *yBegin_;
            }
            Real yMax() const {
                return *(yEnd_-1);
            }
            bool isInRange(Real x, Real y) const {
                return x>=xMin() && x<=xMax() && y>=yMin() && y<=yMax();
            }
          protected:
            Size locateX(Real x) const {
                if (x < *xBegin_)
                    return 0;
                else if (x > *(xEnd_-1))
                    return xEnd_-xBegin_-2;
                else
                    return std::upper_bound(xBegin_,xEnd_-1,x)-xBegin_-1;
            }
            Size locateY(Real y) const {
                if (y < *yBegin_)
                    return 0;
                else if (y > *(yEnd_-1))
                    return yEnd_-yBegin_-2;
                else
                    return std::upper_bound(yBegin_,yEnd_-1,y)-yBegin_-1;
            }
            I1 xBegin_, xEnd_;
            I2 yBegin_, yEnd_;
            const M& zData_;
        };
      public:
        Interpolation2D() {}
        Real operator()(Real x, Real y,
                        bool allowExtrapolation = false) const {
            checkRange(x,y,allowExtrapolation);
            return impl_->value(x,y);
        }
        Real xMin() const { return impl_->xMin(); }
        Real xMax() const { return impl_->xMax(); }
        Real yMin() const { return impl_->yMin(); }
        Real yMax() const { return impl_->yMax(); }
        bool isInRange(Real x, Real y) const {
            return impl_->isInRange(x,y);
        }
      protected:
        void checkRange(Real x, Real y, bool allowExtrapolation) const {
            QL_REQUIRE(allowExtrapolation || impl_->isInRange(x,y),
                       "interpolation range is ["
                       + DecimalFormatter::toString(impl_->xMin()) +
                       ", "
                       + DecimalFormatter::toString(impl_->xMax()) +
                       "] x ["
                       + DecimalFormatter::toString(impl_->yMin()) +
                       ", "
                       + DecimalFormatter::toString(impl_->yMax()) +
                       "]: extrapolation at ("
                       + DecimalFormatter::toString(x) +
                       ", "
                       + DecimalFormatter::toString(y) +
                       " not allowed");
        }
    };

}


#endif
