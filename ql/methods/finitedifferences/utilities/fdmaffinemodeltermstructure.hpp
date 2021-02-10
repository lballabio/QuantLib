/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmaffinemodeltermstructure.hpp
*/

#ifndef quantlib_fdm_affine_model_termstructure_hpp
#define quantlib_fdm_affine_model_termstructure_hpp

#include <ql/math/array.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class AffineModel;

    class FdmAffineModelTermStructure : public YieldTermStructure {
      public:
        FdmAffineModelTermStructure(Array r,
                                    const Calendar& cal,
                                    const DayCounter& dayCounter,
                                    const Date& referenceDate,
                                    const Date& modelReferenceDate,
                                    ext::shared_ptr<AffineModel> model);

        Date maxDate() const override;
        void setVariable(const Array& r);

      protected:
        DiscountFactor discountImpl(Time) const override;

      private:
        Array r_;
        const Time t_;
        const ext::shared_ptr<AffineModel> model_;
    };
}
#endif
