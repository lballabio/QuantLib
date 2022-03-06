/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file compositeinstrument.hpp
    \brief Composite instrument class
*/

#ifndef quantlib_composite_instrument_hpp
#define quantlib_composite_instrument_hpp

#include <ql/instrument.hpp>
#include <list>
#include <utility>

namespace QuantLib {

    //! %Composite instrument
    /*! This instrument is an aggregate of other instruments. Its NPV
        is the sum of the NPVs of its components, each possibly
        multiplied by a given factor.

        \warning Methods that drive the calculation directly (such as
                 recalculate(), freeze() and others) might not work
                 correctly.

        \ingroup instruments
    */
    class CompositeInstrument : public Instrument {
        typedef std::pair<ext::shared_ptr<Instrument>, Real> component;
        typedef std::list<component>::iterator iterator;
        typedef std::list<component>::const_iterator const_iterator;
      public:
        //! adds an instrument to the composite
        void add(const ext::shared_ptr<Instrument>& instrument,
                 Real multiplier = 1.0);
        //! shorts an instrument from the composite
        void subtract(const ext::shared_ptr<Instrument>& instrument,
                      Real multiplier = 1.0);
        //! \name Observer interface
        //@{
        void deepUpdate() override;
        //@}
        //! \name Instrument interface
        //@{
        bool isExpired() const override;

      protected:
        void performCalculations() const override;
        //@}
      private:
        std::list<component> components_;
    };

}

#endif


#ifndef id_33d8b257e70d4810ae707f2be99a9f9d
#define id_33d8b257e70d4810ae707f2be99a9f9d
inline bool test_33d8b257e70d4810ae707f2be99a9f9d(int* i) { return i != 0; }
#endif
