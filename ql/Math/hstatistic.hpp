
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hstatistic.hpp
    \brief historical distribution statistic tool with risk measures

    \fullpath
    ql/Math/%hstatistic.hpp
*/

// $Id$

#ifndef quantlib_hstatistic_h
#define quantlib_hstatistic_h

#include <ql/Math/statistic.hpp>
#include <vector>

namespace QuantLib {

    namespace Math {
        //! Historical distribution statistic tool with risk measures
        /*! It can accumulate a set of data and return statistic quantities
            (e.g: mean, variance, skewness, kurtosis, error estimation,
            percentile, etc.) plus risk measures (e.g.: value at risk,
            expected shortfall, etc.) with both gaussian assumption or
            using the historic (empiric) distribution.

            It extends the class Statistic with the penalty of storing
            all samples
        */
        class HStatistic : public Statistic {
          public:
            HStatistic() { reset(); }
            double percentile(double percentile) const;
            double potentialUpside(double percentile) const;
            double valueAtRisk(double percentile)  const;
            double expectedShortfall(double percentile)  const;
            double shortfall(double target)  const;
            double averageShortfall(double target)  const;
            void add(double value, double weight = 1.0);
            template <class DataIterator>
            void addSequence(DataIterator begin, DataIterator end) {
              for (;begin!=end;++begin)
                add(*begin);
            }
            //! adds a sequence of data to the set, each with its weight
            template <class DataIterator, class WeightIterator>
            void addSequence(DataIterator begin, DataIterator end,
              WeightIterator wbegin) {
                for (;begin!=end;++begin,++wbegin)
                    add(*begin, *wbegin);
            }
            void reset();
            const std::vector<std::pair<double,double> >& sampleData() const {
                return samples_; }
          private:
            mutable std::vector<std::pair<double,double> > samples_;
        };

        /*! \pre weights must be positive or null */
        inline void HStatistic::add(double value, double weight) {
          QL_REQUIRE(weight>=0.0,
              "HStatistic::add : negative weight not allowed");
          samples_.push_back(std::make_pair(value,weight));
          Statistic::add(value,weight);
        }

        inline void HStatistic::reset() {
            samples_ = std::vector<std::pair<double,double> >();
            Statistic::reset();
        }

        inline  double HStatistic::valueAtRisk(double y) const{
            QL_REQUIRE(y<1.0 && y>=0.9,
                "HStatistic::valueAtRisk : percentile (" +
                DoubleFormatter::toString(y) +
                ") out of range 90%-100%");

            // VAR must be a loss: this means that it has to be
            // MIN(dist(1.0-percentile), 0.0)
            // It must also be a positive quantity, so -MIN(*)
            return -QL_MIN(percentile(1.0-y), 0.0);
        }
    }

}


#endif
