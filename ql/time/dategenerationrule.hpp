/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file dategenerationrule.hpp
    \brief date generation rule
*/

#ifndef quantlib_date_generation_rule_hpp
#define quantlib_date_generation_rule_hpp

#include <ql/qldefines.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Date-generation rule
    /*! These conventions specify the rule used to generate dates in a
        Schedule.

        \ingroup datetime
    */
    struct DateGeneration {
        enum Rule {
            Backward,       /*!< Backward from termination date to
                                 effective date. */
            Forward,        /*!< Forward from effective date to
                                 termination date. */
            Zero,           /*!< No intermediate dates between effective date
                                 and termination date. */
            ThirdWednesday,  /*!< All dates but effective date and termination
                                  date are taken to be on the third wednesday
                                  of their month (with forward calculation. ) */
            ThirdWednesdayInclusive, /*!< All dates including effective date and termination
                                  date are taken to be on the third wednesday
                                  of their month (with forward calculation) */
            Twentieth,      /*!< All dates but the effective date are
                                 taken to be the twentieth of their
                                 month (used for CDS schedules in
                                 emerging markets.)  The termination
                                 date is also modified. */
            TwentiethIMM,   /*!< All dates but the effective date are
                                 taken to be the twentieth of an IMM
                                 month (used for CDS schedules.)  The
                                 termination date is also modified. */
            OldCDS,         /*!< Same as TwentiethIMM with unrestricted date
                                 ends and log/short stub coupon period (old
                                 CDS convention). */
            CDS,             /*!< Credit derivatives standard rule since 'Big
                               Bang' changes in 2009.  */
            CDS2015,         /*!< Credit derivatives standard rule since
                               December 20th, 2015.  */
        };
    };

    /*! \relates DateGeneration */
    std::ostream& operator<<(std::ostream&,
                             DateGeneration::Rule);

}

#endif
