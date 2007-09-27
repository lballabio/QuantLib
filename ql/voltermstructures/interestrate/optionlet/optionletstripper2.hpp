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

    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapFloorMatrix;

    class OptionletStripper2 : public LazyObject {
      public:
        OptionletStripper2(const Handle<OptionletStripper>& optionletStripper,
                           const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve,
                           const std::vector<Rate>& atmStrikes,
                           Real alpha = 0.2,
                           Real beta = 0.6,
                           Real nu = 0.02,
                           Real rho = 0.,
                           bool isAlphaFixed = false,
                           bool isBetaFixed = true,
                           bool isNuFixed = false,
                           bool isRhoFixed = false,
                           bool vegaWeighted = false,
                           const boost::shared_ptr<EndCriteria>& endCriteria
                            = boost::shared_ptr<EndCriteria>(),
                           const boost::shared_ptr<OptimizationMethod>& method
                            = boost::shared_ptr<OptimizationMethod>());

        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        //@}
        std::vector<double> spreadsVol() const;
        std::vector<double> atmOptionPrice() const;
        std::vector<double> mdlOptionletVols(Size i) const; 

      private:

        std::vector<double> spreadsVolImplied() const;

        class ObjectiveFunction {
          public:
            ObjectiveFunction(const std::vector<boost::shared_ptr<CapFloor> >& caplets,
                              const std::vector<double>& modVols,
                              const DayCounter& dc,
                              Real targetValue);
            Real operator()(Volatility spreadVol) const;
          private:
            std::vector<boost::shared_ptr<CapFloor> > caplets_;
            std::vector<double> modVols_;
            DayCounter dc_;
            Real targetValue_;

        };
        
        const Handle<OptionletStripper> optionletStripper_;
        const Handle<CapFloorTermVolCurve> atmCapFloorTermVolCurve_;
        DayCounter dc_;
        Size nOptionExpiries_;
        const std::vector<Rate> atmStrikes_;
        mutable std::vector<Real> atmOptionPrice_;
        mutable std::vector<std::vector<double> > mdlOptionletVols_;
        mutable std::vector<double> spreadsVolImplied_;
        mutable std::vector<std::vector<double> > calibratedOptionletVols_;
        mutable CapFloorMatrix caplets_;
        mutable std::vector<boost::shared_ptr<CapFloor> > caps_;
        Size maxEvaluations_;
        Real accuracy_;

        Real alpha_, beta_, nu_, rho_;
        bool isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_;
        bool vegaWeighted_;
        boost::shared_ptr<EndCriteria> endCriteria_;
        boost::shared_ptr<OptimizationMethod> method_;
    };

 
}

#endif
