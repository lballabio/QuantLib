/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andrea Odetti

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

#ifndef quantlib_longstaff_schwartz_multi_path_pricer_hpp
#define quantlib_longstaff_schwartz_multi_path_pricer_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/methods/montecarlo/pathpricer.hpp>
#include <ql/methods/montecarlo/multipath.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/experimental/mcbasket/pathpayoff.hpp>
#include <ql/functional.hpp>
#include <memory>

namespace QuantLib {

    //! Longstaff-Schwarz path pricer for early exercise options
    /*! References:

        Francis Longstaff, Eduardo Schwartz, 2001. Valuing American Options
        by Simulation: A Simple Least-Squares Approach, The Review of
        Financial Studies, Volume 14, No. 1, 113-147

        \ingroup mcarlo

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */
    class LongstaffSchwartzMultiPathPricer : public PathPricer<MultiPath> {
      public:
        LongstaffSchwartzMultiPathPricer(const ext::shared_ptr<PathPayoff>& payoff,
                                         const std::vector<Size>& timePositions,
                                         std::vector<Handle<YieldTermStructure> > forwardTermStructure,
                                         Array discounts,
                                         Size polynomialOrder,
                                         LsmBasisSystem::PolynomialType polynomialType);

        Real operator()(const MultiPath& multiPath) const override;
        virtual void calibrate();

      protected:
        struct PathInfo {
            explicit PathInfo(Size numberOfTimes);

            Size pathLength() const;

            Array                   payments;
            Array                   exercises;
            std::vector<Array>      states;
        };

        PathInfo transformPath(const MultiPath& path) const;

        bool calibrationPhase_ = true;

        const ext::shared_ptr<PathPayoff> payoff_;

        std::unique_ptr<Array[]> coeff_;
        std::unique_ptr<Real[]> lowerBounds_;

        const std::vector<Size> timePositions_;
        const std::vector<Handle<YieldTermStructure> > forwardTermStructures_;
        const Array dF_;

        mutable std::vector<PathInfo> paths_;
        const std::vector<ext::function<Real(Array)> > v_;
    };

}


#endif
