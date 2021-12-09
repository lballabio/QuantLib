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

/*! \file adaptedpathpayoff.hpp
    \brief Adapted Option payoff classes
*/

#ifndef quantlib_adapted_path_payoff_hpp
#define quantlib_adapted_path_payoff_hpp

#include <ql/experimental/mcbasket/pathpayoff.hpp>

namespace QuantLib {

    class AdaptedPathPayoff : public PathPayoff {
    public:

        class ValuationData {
        public:
            Size numberOfTimes() const;
            Size numberOfAssets() const;

            Real getAssetValue(Size time, Size asset);

            const Handle<YieldTermStructure> & getYieldTermStructure(Size time);

            void setPayoffValue(Size time, Real value);

            // set the exercise payoff if the option is exercised at time "time"
            // it destroys state
            void setExerciseData(Size time, Real exercise, Array & state);

        private:

            friend class AdaptedPathPayoff;

            ValuationData(const Matrix       & path, 
                          const std::vector<Handle<YieldTermStructure> > & forwardTermStructures,
                          Array              & payments, 
                          Array              & exercises, 
                          std::vector<Array> & states);

            const Matrix & path_;
            const std::vector<Handle<YieldTermStructure> > & forwardTermStructures_;

            Array & payments_;
            Array & exercises_;
            std::vector<Array> & states_;

            Size maximumTimeRead_;
        };

        void value(const Matrix& path,
                   const std::vector<Handle<YieldTermStructure> >& forwardTermStructures,
                   Array& payments,
                   Array& exercises,
                   std::vector<Array>& states) const override;

      protected:

        /*
          Key method of this class.

          It computes the payoff by calling ValuationData::setPayoffValue();
          and provides early exercise information 
          via ValuationData::setExerciseData();
         */
        virtual void operator()(ValuationData & data) const = 0;
    };
}

#endif
