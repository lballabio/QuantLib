/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2021 Ralf Konrad Eckel

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

/*! \file inflationindex.hpp
    \brief base classes for inflation indexes
*/

#ifndef quantlib_inflation_index_hpp
#define quantlib_inflation_index_hpp

#include <ql/currency.hpp>
#include <ql/handle.hpp>
#include <ql/index.hpp>
#include <ql/indexes/region.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>

namespace QuantLib {

    class ZeroInflationIndex;

    struct CPI {
        //! when you observe an index, how do you interpolate between fixings?
        enum InterpolationType {
            AsIndex, //!< same interpolation as index
            Flat,    //!< flat from previous fixing
            Linear   //!< linearly between bracketing fixings
        };

        //! interpolated inflation fixing
        /*! \param index              The index whose fixing should be retrieved
            \param date               The date without lag; usually, the payment
                                      date for some inflation-based coupon.
            \param observationLag     The observation lag to be subtracted from the
                                      passed date; for instance, if the passed date is
                                      in May and the lag is three months, the inflation
                                      fixing from February (and March, in case of
                                      interpolation) will be observed.
            \param interpolationType  The interpolation type (flat or linear)
        */
        static Real laggedFixing(const ext::shared_ptr<ZeroInflationIndex>& index,
                                 const Date& date,
                                 const Period& observationLag,
                                 InterpolationType interpolationType);
    };


    //! Base class for inflation-rate indexes,
    class InflationIndex : public Index {
      public:
        InflationIndex(std::string familyName,
                       Region region,
                       bool revised,
                       Frequency frequency,
                       const Period& availabilitiyLag,
                       Currency currency);

        //! \name Index interface
        //@{
        std::string name() const override;

        /*! Inflation indices do not have fixing calendars.  An
            inflation index value is valid for every day (including
            weekends) of a calendar period.  I.e. it uses the
            NullCalendar as its fixing calendar.
        */
        Calendar fixingCalendar() const override;
        bool isValidFixingDate(const Date&) const override { return true; }

        /*! Forecasting index values requires an inflation term
            structure.  The inflation term structure (ITS) defines the
            usual lag (not the index).  I.e.  an ITS is always relatve
            to a base date that is earlier than its asof date.  This
            must be so because indices are available only with a lag.
            However, the index availability lag only sets a minimum
            lag for the ITS.  An ITS may be relative to an earlier
            date, e.g. an index may have a 2-month delay in
            publication but the inflation swaps may take as their base
            the index 3 months before.
        */
        Real fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override = 0;

        //! returns a past fixing at the given date
        Real pastFixing(const Date& fixingDate) const override = 0;

        /*! this method creates all the "fixings" for the relevant
            period of the index.  E.g. for monthly indices it will put
            the same value in every calendar day in the month.
        */
        void addFixing(const Date& fixingDate, Rate fixing, bool forceOverwrite = false) override;
        //@}

        //! \name Inspectors
        //@{
        std::string familyName() const;
        Region region() const;
        bool revised() const;
        /*! Forecasting index values using an inflation term structure
            uses the interpolation of the inflation term structure
            unless interpolation is set to false.  In this case the
            extrapolated values are constant within each period taking
            the mid-period extrapolated value.
        */

        Frequency frequency() const;
        /*! The availability lag describes when the index is
            <i>available</i>, not how it is used.  Specifically the
            fixing for, say, January, may only be available in April
            but the index will always return the index value
            applicable for January as its January fixing (independent
            of the lag in availability).
        */
        Period availabilityLag() const;
        Currency currency() const;
        //@}

      protected:
        Date referenceDate_;
        std::string familyName_;
        Region region_;
        bool revised_;
        Frequency frequency_;
        Period availabilityLag_;
        Currency currency_;

      private:
        std::string name_;
    };


    //! Base class for zero inflation indices.
    class ZeroInflationIndex : public InflationIndex {
      public:
        ZeroInflationIndex(
            const std::string& familyName,
            const Region& region,
            bool revised,
            Frequency frequency,
            const Period& availabilityLag,
            const Currency& currency,
            Handle<ZeroInflationTermStructure> ts = {});

        //! \name Index interface
        //@{
        /*! \warning the forecastTodaysFixing parameter (required by
                     the Index interface) is currently ignored.
        */
        Real fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override;
        //! returns a past fixing at the given date
        Real pastFixing(const Date& fixingDate) const override;
        //@}
        //! \name Other methods
        //@{
        Date lastFixingDate() const;
        Handle<ZeroInflationTermStructure> zeroInflationTermStructure() const;
        ext::shared_ptr<ZeroInflationIndex> clone(const Handle<ZeroInflationTermStructure>& h) const;
        //@}
      private:
        bool needsForecast(const Date& fixingDate) const;
        Real forecastFixing(const Date& fixingDate) const;
        Handle<ZeroInflationTermStructure> zeroInflation_;
    };


    //! Base class for year-on-year inflation indices.
    /*! These may be quoted indices published on, say, Bloomberg, or can be
        defined as the ratio of an index at different time points.
    */
    class YoYInflationIndex : public InflationIndex {
      public:
        //! \name Constructors
        //@{
        //! Constructor for year-on-year indices defined as a ratio.
        /*! An index build with this constructor doesn't need to store
            past fixings of its own; they will be calculated as a
            ratio from the past fixings stored in the underlying index.
        */
        YoYInflationIndex(
            const ext::shared_ptr<ZeroInflationIndex>& underlyingIndex,
            bool interpolated,
            Handle<YoYInflationTermStructure> ts = {});

        //! Constructor for quoted year-on-year indices.
        /*! An index built with this constructor needs its past
            fixings (i.e., the past year-on-year values) to be stored
            via the `addFixing` or `addFixings` method.
        */
        YoYInflationIndex(
            const std::string& familyName,
            const Region& region,
            bool revised,
            bool interpolated,
            Frequency frequency,
            const Period& availabilityLag,
            const Currency& currency,
            Handle<YoYInflationTermStructure> ts = {});

        //@}

        //! \name Index interface
        //@{
        /*! \warning the forecastTodaysFixing parameter (required by
                     the Index interface) is currently ignored.
        */
        Rate fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override;

        /*! returns a past fixing at the given date
         *  \warning This is only supported for flat YOY indices providing their own timeseries
         *           via the `addFixing` or `addFixings` method,
         *           aka where ratio() == interpolated() == false.
         */
        Real pastFixing(const Date& fixingDate) const override;
        //@}
        //! \name Other methods
        //@{
        // Override the deprecation above
        bool interpolated() const;
        bool ratio() const;
        ext::shared_ptr<ZeroInflationIndex> underlyingIndex() const;
        Handle<YoYInflationTermStructure> yoyInflationTermStructure() const;

        ext::shared_ptr<YoYInflationIndex> clone(const Handle<YoYInflationTermStructure>& h) const;
        //@}

      protected:
        // Override the deprecation above
        bool interpolated_;

      private:
        Rate forecastFixing(const Date& fixingDate) const;
        bool ratio_;
        ext::shared_ptr<ZeroInflationIndex> underlyingIndex_;
        Handle<YoYInflationTermStructure> yoyInflation_;
    };


    namespace detail::CPI {
            // Returns either CPI::Flat or CPI::Linear depending on the combination of index and
            // CPI::InterpolationType.
            QuantLib::CPI::InterpolationType effectiveInterpolationType(
                const QuantLib::CPI::InterpolationType& type = QuantLib::CPI::AsIndex);


            // checks whether the combination of index and CPI::InterpolationType results
            // effectively in CPI::Linear
            bool isInterpolated(const QuantLib::CPI::InterpolationType& type = QuantLib::CPI::AsIndex);
        }


    // inline

    inline std::string InflationIndex::name() const {
        return name_;
    }

    inline std::string InflationIndex::familyName() const {
        return familyName_;
    }

    inline Region InflationIndex::region() const {
        return region_;
    }

    inline bool InflationIndex::revised() const {
        return revised_;
    }

    inline Frequency InflationIndex::frequency() const {
        return frequency_;
    }

    inline Period InflationIndex::availabilityLag() const {
        return availabilityLag_;
    }

    inline Currency InflationIndex::currency() const {
        return currency_;
    }

    inline Handle<ZeroInflationTermStructure>
    ZeroInflationIndex::zeroInflationTermStructure() const {
        return zeroInflation_;
    }

    inline bool YoYInflationIndex::interpolated() const {
        return interpolated_;
    }

    inline bool YoYInflationIndex::ratio() const {
        return ratio_;
    }

    inline ext::shared_ptr<ZeroInflationIndex> YoYInflationIndex::underlyingIndex() const {
        return underlyingIndex_;
    }

    inline Handle<YoYInflationTermStructure>
    YoYInflationIndex::yoyInflationTermStructure() const {
        return yoyInflation_;
    }

    inline bool detail::CPI::isInterpolated(const QuantLib::CPI::InterpolationType& type) {
        return detail::CPI::effectiveInterpolationType(type) == QuantLib::CPI::Linear;
    }

}

#endif
