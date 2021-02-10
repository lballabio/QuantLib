/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

/*! \file forwardvaluequote.hpp
    \brief quote for the forward value of an index
*/

#ifndef quantlib_forward_value_quote_hpp
#define quantlib_forward_value_quote_hpp

#include <ql/quote.hpp>
#include <ql/index.hpp>

namespace QuantLib {

    //! %quote for the forward value of an index
    class ForwardValueQuote : public Quote,
                              public Observer {
      public:
        ForwardValueQuote(ext::shared_ptr<Index> index, const Date& fixingDate);
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
        void update() override;

      private:
        ext::shared_ptr<Index> index_;
        Date fixingDate_;
    };

}


#endif
