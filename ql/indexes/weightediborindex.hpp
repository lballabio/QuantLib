/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

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

/*! \file weightediborindex.hpp
    \brief index fixing as a fixed-weight combination of Ibor indices
*/

#ifndef quantlib_weightediborindex_hpp
#define quantlib_weightediborindex_hpp

#include <ql/indexes/iborindex.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! %Index fixing as a fixed-weight linear combination of Ibor indices
    /*! Like SwapSpreadIndex, this index has no fixings and no forwarding
        curve of its own (it delegates both to its components).

        The components must share their conventions (fixing days, fixing
        calendar, day counter, currency). Only their tenors differ, and
        the maturity of the combination is that of the longest component.
    */
    class WeightedIborIndex : public IborIndex {
      public:
        //! a component index and the weight it carries in the fixing
        using Component = std::pair<ext::shared_ptr<IborIndex>, Real>;

        explicit WeightedIborIndex(std::vector<Component> components);

        //! \name InterestRateIndex interface
        //@{
        //! the maturity of the longest component
        Date maturityDate(const Date& valueDate) const override;
        Rate forecastFixing(const Date& fixingDate) const override;
        Rate pastFixing(const Date& fixingDate) const override;
        bool allowsNativeFixings() override { return false; }
        //@}

        //! \name IborIndex interface
        //@{
        /*! \warning a combination of indices cannot be relinked to a single
                     curve (exception thrown). Relink the components instead.
        */
        ext::shared_ptr<IborIndex> clone(const Handle<YieldTermStructure>&) const override;
        //@}

        //! \name Inspectors
        //@{
        const std::vector<Component>& components() const { return components_; }
        //@}

      private:
        static const ext::shared_ptr<IborIndex>&
        firstComponent(const std::vector<Component>& components);
        static Period longestTenor(const std::vector<Component>& components);

        std::vector<Component> components_;
    };

}

#endif
