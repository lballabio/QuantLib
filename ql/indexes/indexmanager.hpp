/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

/*! \file indexmanager.hpp
    \brief global repository for past index fixings
*/

#ifndef quantlib_index_manager_hpp
#define quantlib_index_manager_hpp

#include <ql/timeseries.hpp>
#include <ql/patterns/singleton.hpp>
#include <ql/utilities/observablevalue.hpp>


namespace QuantLib {

    //! global repository for past index fixings
    /*! \note index names are case insensitive */
    class IndexManager : public Singleton<IndexManager> {
        friend class Singleton<IndexManager>;
      private:
        IndexManager() {}
      public:
        //! returns whether historical fixings were stored for the index
        bool hasHistory(const std::string& name) const;
        //! returns the (possibly empty) history of the index fixings
        const TimeSeries<Real>& getHistory(const std::string& name) const;
        //! stores the historical fixings of the index
        void setHistory(const std::string& name, const TimeSeries<Real>&);
        //! observer notifying of changes in the index fixings
        boost::shared_ptr<Observable> notifier(const std::string& name) const;
        //! returns all names of the indexes for which fixings were stored
        std::vector<std::string> histories() const;
        //! clears the historical fixings of the index
        void clearHistory(const std::string& name);
        //! clears all stored fixings
        void clearHistories();
      private:
        typedef std::map<std::string, ObservableValue<TimeSeries<Real> > >
                                                                  history_map;
        mutable history_map data_;
    };

}


#endif
