/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file optionletstripper2.hpp
    \brief optionlet (cap/floor) volatility stripper
*/

#ifndef quantlib_optionletstripper2_hpp
#define quantlib_optionletstripper2_hpp

#include <ql/voltermstructures/interestrate/optionlet/optionletstripper.hpp>
#include <ql/voltermstructures/sabrinterpolatedsmilesection.hpp>

namespace QuantLib {

    class CapFloorTermVolCurve;

    class OptionletStripper2 : public LazyObject {
      public:
        OptionletStripper2(const Handle<OptionletStripper>& optionletStripper,
                           const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve);

        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        //@}
        std::vector<Volatility> spreadsVol() const;
        std::vector<Rate> atmOptionStrikes() const;
        std::vector<Real> atmOptionPrices() const;
        std::vector<Volatility> mdlOptionletVols(Size i) const; 

      private:

        std::vector<Volatility> spreadsVolImplied() const;

        class ObjectiveFunction {
          public:
            ObjectiveFunction(const Handle<OptionletStripper>& optionletStripper,
                              const boost::shared_ptr<CapFloor>& cap,
                              Real targetValue);
            Real operator()(Volatility spreadVol) const;
          private:
            const Handle<OptionletStripper> optionletStripper_;
            boost::shared_ptr<CapFloor> cap_;
            Real targetValue_;

        };
        
        const Handle<OptionletStripper> optionletStripper_;
        const Handle<CapFloorTermVolCurve> atmCapFloorTermVolCurve_;
        DayCounter dc_;
        Size nOptionExpiries_;
        mutable std::vector<Rate> atmStrikes_;
        mutable std::vector<Real> atmOptionPrice_;
        mutable std::vector<Volatility> spreadsVolImplied_;
        mutable std::vector<boost::shared_ptr<CapFloor> > caps_;
        Size maxEvaluations_;
        Real accuracy_;
    };

 
}

#endif
