
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

/*! \file discrepancystatistic.hpp
    \brief Statistic tool for sequences with discrepancy calculation

    \fullpath
    ql/Math/%discrepancystatistic.hpp
*/

// $Id$

#ifndef quantlib_dicrepancy_statistic_hpp
#define quantlib_dicrepancy_statistic_hpp

#include <ql/Math/sequencestatistic.hpp>

namespace QuantLib {

    namespace Math {

        //! Statistic tool for sequences with discrepancy calculation
        /*! It inherit from SequenceStatistic<HStatistic> and adds
            \f$ L^2 \f$ discrepancy calculation
        */
        template <class SequenceType>
        class DiscrepancyStatistic : public SequenceStatistic<SequenceType,
                                                              HStatistic> {
          public:
            // typedefs
            typedef SequenceType   sequence_type;
            // constructor
            DiscrepancyStatistic(Size dimension)
            : SequenceStatistic<SequenceType, HStatistic>(dimension) {
                reset(dimension); }
            //! \name 1-dimensional inspectors
            //@{
            double discrepancy() const;
            //@}
            void add(const DiscrepancyStatistic<SequenceType>::sequence_type& sample,
                     double weight = 1.0);
          void reset(Size dimension);
          private:
            mutable double adiscr_, cdiscr_;
            double bdiscr_, ddiscr_;
        };

        template <class Seq>
        void DiscrepancyStatistic<Seq>::reset(Size dimension) {
            SequenceStatistic<Seq, HStatistic>::reset(dimension);
            adiscr_ = 0.0;
            bdiscr_ = 1.0/QL_POW(2, dimension-1);
            cdiscr_ = 0.0;
            ddiscr_ = 1.0/QL_POW(3, dimension);
        }

        template <class Seq>
        void DiscrepancyStatistic<Seq>::add(
          const DiscrepancyStatistic<Seq>::sequence_type& sample,
          double weight) {
            SequenceStatistic<Seq, HStatistic>::add(sample, weight);

            Size k, m, N = samples();

            double r_ik, r_jk, temp = 1.0;
            for (k=0; k<dimension_; k++) {
                r_ik = sample[k]; //i=N
                temp *= (1.0 - r_ik*r_ik);
            }
            cdiscr_ += temp;

            for (m=0; m<N-1; m++) {
                temp = 1.0;
                for (k=0; k<dimension_; k++) {
                    // running i=1..(N-1)
                    r_ik = stats_[k].sampleData()[m].first;
                    // fixed j=N
                    r_jk = sample[k];
                    temp *= (1.0 - QL_MAX(r_ik, r_jk));
                }
                adiscr_ += temp;

                temp = 1.0;
                for (k=0; k<dimension_; k++) {
                    // fixed i=N
                    r_ik = sample[k];
                    // running j=1..(N-1)
                    r_jk = stats_[k].sampleData()[m].first;
                    temp *= (1.0 - QL_MAX(r_ik, r_jk));
                }
                adiscr_ += temp;
            }
            temp = 1.0;
            for (k=0; k<dimension_; k++) {
                // fixed i=N
                r_ik = sample[k];
                // fixed j=N
                r_jk = sample[k];
                temp *= (1.0 - QL_MAX(r_ik, r_jk));
            }
            adiscr_ += temp;
        }

        template <class Seq>
        double DiscrepancyStatistic<Seq>::discrepancy() const {
            Size N = samples();
/*
            Size i;
            double r_ik, r_jk, cdiscr = adiscr = 0.0, temp = 1.0;

            for (i=0; i<N; i++) {
                double temp = 1.0;
                for (Size k=0; k<dimension_; k++) {
                    r_ik = stats_[k].sampleData()[i].first;
                    temp *= (1.0 - r_ik*r_ik);
                }
                cdiscr += temp;
            }

            for (i=0; i<N; i++) {
                for (Size j=0; j<N; j++) {
                    double temp = 1.0;
                    for (Size k=0; k<dimension_; k++) {
                        r_jk = stats_[k].sampleData()[j].first;
                        r_ik = stats_[k].sampleData()[i].first;
                        temp *= (1.0 - QL_MAX(r_ik, r_jk));
                    }
                    adiscr += temp;
                }
            }
*/

            return QL_SQRT(adiscr_/(N*N)-bdiscr_/N*cdiscr_+ddiscr_);
        }

        typedef DiscrepancyStatistic<std::vector<double> > DiscrepancyVectorStatistic;
        typedef DiscrepancyStatistic<Array> DiscrepancyArrayStatistic;
    }

}


#endif
