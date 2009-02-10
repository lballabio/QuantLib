/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl

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

/*! \file issuer.hpp
    \brief Classes for credit-name handling.
*/

#ifndef quantlib_issuer_hpp
#define quantlib_issuer_hpp

#include <ql/default.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    class Issuer {
      public:
        Issuer(const Handle<DefaultProbabilityTermStructure>& probability =
                                    Handle<DefaultProbabilityTermStructure>(),
               Real recoveryRate = 0.4,
               const std::vector<boost::shared_ptr<DefaultEvent> >& events =
                             std::vector<boost::shared_ptr<DefaultEvent> >());
        //! \name Inspectors
        //@{
        //! default probability
        const Handle<DefaultProbabilityTermStructure>&
        defaultProbability() const;
        //! expected recovery rate
        Real recoveryRate() const;
        //@}

        //! \name Utilities
        //@{
        /*! If a default event with the required seniority and
            restructuring type is found, it is returned for
            inspection; otherwise, the method returns an empty pointer.
        */
        boost::shared_ptr<DefaultEvent>
        defaultedBetween(const Date& start,
                         const Date& end,
                         Seniority seniority = AnySeniority,
                         Restructuring restructuring = AnyRestructuring) const;
        //@}

        //! \name Observability
        //@{
        operator boost::shared_ptr<Observable>() const;
        //@}

      private:
        Handle<DefaultProbabilityTermStructure> probability_;
        Real recoveryRate_;
        std::vector<boost::shared_ptr<DefaultEvent> > events_;
    };

}


#endif

