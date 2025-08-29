/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discrepancystatistics.hpp
    \brief Statistic tool for sequences with discrepancy calculation
*/

#ifndef quantlib_dicrepancy_statistics_hpp
#define quantlib_dicrepancy_statistics_hpp

#include <ql/math/statistics/sequencestatistics.hpp>

namespace QuantLib {

    //! Statistic tool for sequences with discrepancy calculation
    /*! It inherit from SequenceStatistics<Statistics> and adds
        \f$ L^2 \f$ discrepancy calculation
    */
    class DiscrepancyStatistics : public SequenceStatistics {
      public:
        typedef SequenceStatistics::value_type value_type;
        // constructor
        DiscrepancyStatistics(Size dimension);
        //! \name 1-dimensional inspectors
        //@{
        Real discrepancy() const;
        //@}
        template <class Sequence>
        void add(const Sequence& sample,
                 Real weight = 1.0) {
            add(sample.begin(),sample.end(),weight);
        }
        template <class Iterator>
        void add(Iterator begin,
                 Iterator end,
                 Real weight = 1.0) {
            SequenceStatistics::add(begin,end,weight);

            Size k, m, N = samples();

            Real r_ik, r_jk, temp = 1.0;
            Iterator it;
            for (k=0, it=begin; k<dimension_; ++it, ++k) {
                r_ik = *it; //i=N
                temp *= (1.0 - r_ik*r_ik);
            }
            cdiscr_ += temp;

            for (m=0; m<N-1; m++) {
                temp = 1.0;
                for (k=0, it=begin; k<dimension_; ++it, ++k) {
                    // running i=1..(N-1)
                    r_ik = stats_[k].data()[m].first;
                    // fixed j=N
                    r_jk = *it;
                    temp *= (1.0 - std::max(r_ik, r_jk));
                }
                adiscr_ += temp;

                temp = 1.0;
                for (k=0, it=begin; k<dimension_; ++it, ++k) {
                    // fixed i=N
                    r_ik = *it;
                    // running j=1..(N-1)
                    r_jk = stats_[k].data()[m].first;
                    temp *= (1.0 - std::max(r_ik, r_jk));
                }
                adiscr_ += temp;
            }
            temp = 1.0;
            for (k=0, it=begin; k<dimension_; ++it, ++k) {
                // fixed i=N, j=N
                r_ik = r_jk = *it;
                temp *= (1.0 - std::max(r_ik, r_jk));
            }
            adiscr_ += temp;
        }
        void reset(Size dimension = 0);
      private:
        mutable Real adiscr_, cdiscr_;
        Real bdiscr_, ddiscr_;
    };


    // inline definitions

    inline DiscrepancyStatistics::DiscrepancyStatistics(Size dimension)
    : SequenceStatistics(dimension) {
        reset(dimension);
    }

    inline void DiscrepancyStatistics::reset(Size dimension) {
        if (dimension == 0)           // if no size given,
            dimension = dimension_;   // keep the current one
        QL_REQUIRE(dimension != 1,
                   "dimension==1 not allowed");

        SequenceStatistics::reset(dimension);

        adiscr_ = 0.0;
        bdiscr_ = 1.0/std::pow(2.0, Integer(dimension-1));
        cdiscr_ = 0.0;
        ddiscr_ = 1.0/std::pow(3.0, Integer(dimension));
    }

}


#endif
