/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*!
Copyright (C) 2023 Oliver Ofenloch

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

/* This example reads a QuantLib::TimeSeries<YahhoQuote> from a 
   given CSV file. The class YahhoQuote is defined in the header
   file YahooQuot.hpp in this directory. An example CSV file is also
   provided in this directory.

   Once the TimeSeries is read from the CSV file, some simple calculations
   are performed.

   Do soemthing like this

   ./build/Examples/TimeSeriesFromCSV/TimeSeriesFromCSV ./Examples/TimeSeriesFromCSV/example.csv 

   to run the example.
*/

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <ctype.h>
#include <exception>
#include <fstream>
#include <istream>
#include <map>
#include <ql/quantlib.hpp>
#include <string>

#include "YahooQuote.hpp"

using namespace QuantLib;

#define MAX_LINE_LENGTH 1024

TimeSeries<YahooQuote> ReadFromCSV(boost::filesystem::path const& filename) {
    // Read the file provided via command line
    std::ifstream in(filename.c_str());
    char line[MAX_LINE_LENGTH];
    std::vector<std::string> lines;
    while (in.getline(line, MAX_LINE_LENGTH, '\n')) {
        lines.push_back(line);
    }

    // Create a TimeSeries object
    TimeSeries<YahooQuote> series{};

    for (unsigned int i = 0; i < lines.size(); i++) {
        if (!std::isdigit(lines[i].at(0))) {
            // Ignore header and comment lines.
            // The lines look like this:
            //    2000-01-03,19.900000,20.400000,19.799999,20.000000,14.064997,6400
            // Usually the files has a header line like this:
            //    Date,Open,High,Low,Close,Adj Close,Volume
            // We'd like to be able to comment some lines with '#'.
            //
            // So, at least for now, we simply ignore all lines that do not start with a digit.
            continue;
        }
        std::vector<std::string> outerArray;

        boost::split(outerArray, lines[i], boost::is_any_of(","));

        std::vector<std::string> innerArray;
        boost::split(innerArray, outerArray[0], boost::is_any_of("-"));

        Year year = (Year)std::stoi(innerArray[0]);
        Month month = (Month)std::stoi(innerArray[1]);
        Day day = (Day)std::stoi(innerArray[2]);
        Date date(day, month, year);
        auto parseReal = [](std::string const& s) -> Real {
            // Sometimes there are lines like
            //   1988-06-17,null,null,null,null,null,null
            // We make sure the "null" strings become proper Null<Real>() values.
            if (std::isdigit(s.at(0))) {
                return atof(s.c_str());
            } else {
                return Null<Real>();
            }
        }; // end of lambda
        Real open = parseReal(outerArray[1]);
        Real high = parseReal(outerArray[2]);
        Real low = parseReal(outerArray[3]);
        Real close = parseReal(outerArray[4]);
        Real adjclose = parseReal(outerArray[5]);
        Real volume = parseReal(outerArray[6]);

        YahooQuote yquote(date, open, high, low, close, adjclose, volume);
        series[yquote.date()] = yquote;
    }
    // Return the time series
    return series;
}

int main(int argc, char* argv[]) {

    try {

        if (argc != 2) {
            std::cout << "usage: " << argv[0] << " filename" << std::endl;
            std::cout << "       reads data from CSV file into a TimeSeries<double>" << std::endl;
            return 1;
        }
        // Source file
        boost::filesystem::path csv_file{argv[1]};

        // Call to the function
        TimeSeries<YahooQuote> mySeries = ReadFromCSV(csv_file);

        // Is the time series empty?
        if (mySeries.empty()) {
            std::cout << "TimeSeries is empty.\n";
            return EXIT_FAILURE;
        }
        std::cout << "TimeSeries has " << mySeries.size() << " entries.\n";

        const Date dFirstDate = mySeries.firstDate();
        const Date dLastDate = mySeries.lastDate();
        const Date dBeforeLastDate = mySeries.lastDate() - 1;

        // Start date of the time series
        std::cout << "TimeSeries' first date is " << dFirstDate << '\n';

        // Last date of the time series
        std::cout << "TimeSeries' last date is " << dLastDate << '\n';

        // What was the Adj.close value on the day befor the last day?
        std::cout << "Adjusted close on " << dBeforeLastDate << " was "
                  << mySeries[dBeforeLastDate].adj_close() << '\n';

        // What was the Adj.close value on the last day?
        std::cout << "Adjusted close on " << dBeforeLastDate << " was "
                  << mySeries[dLastDate].adj_close() << '\n';

        // Do some basic calculations ...
        std::multimap<Real, Date> delta_adj_close;
        std::multimap<Real, Date> delta_adj_close_rel;
        typedef std::multimap<Real, Date>::iterator multimap_iterator;
        std::ofstream data1("example-data1.dat");
        data1 << "# date delta delta_rel\n";
        for (auto date : mySeries.dates()) {
            auto yqoute = mySeries[date];
            // today's price S(t)
            Real today = yqoute.adj_close();
            if (today == Null<Real>()) {
                // ignore null values
                continue;
            }
            // yesterday's price S(t-1)
            Real yesterday = mySeries[date - 1].adj_close();
            if (yesterday == Null<Real>()) {
                // ignore null values
                continue;
            }
            // price increment deltaS(t) = S(t) - S(t-1)
            Real delta = today - yesterday;
            Real delta_rel{Null<Real>()};
            if (yesterday) {
                // relative price increment deltaS(t) = ( S(t) - S(t-1) ) / S(t-1)
                delta_rel = delta / yesterday;
            }
            yqoute.set_daily_return_abs(delta);
            yqoute.set_daily_return_rel(delta_rel);
            delta_adj_close.insert(decltype(delta_adj_close)::value_type(delta, date));
            delta_adj_close_rel.insert(decltype(delta_adj_close_rel)::value_type(delta_rel, date));
            data1 << io::iso_date(date) << " " << delta << " " << delta_rel * 100.0 << "\n";
        }
        data1.close();
        // plot with (if you don't have gnuplot please figure out something else...)
        //   gnuplot -e "set terminal png ; set xdata time ;set timefmt \"%Y-%m-%d\" ; set output 'example-data1.png' ; plot 'example-data1.dat' using 1:3 w l"
        //

        //
        // As far as I know there's no way to the stuff below with a TimeSeries.
        //

        // get the smallest and largest price increment (return)
        double smallest_rel_return = delta_adj_close_rel.begin()->first;
        double larget_rel_return = delta_adj_close_rel.rbegin()->first;
        std::cout << "Smallest relative daily return : " << smallest_rel_return * 100.0 << " %\n";
        std::cout << "Largest relative daily return  :  " << larget_rel_return * 100.0 << " %\n";

        // make 100 return intervals
        const size_t n_intervals = mySeries.size() / 100;
        Real interval_width = (larget_rel_return - smallest_rel_return) / n_intervals;
        std::vector<Real> interval_bounds;
        for (size_t i = 0; i < n_intervals; i++) {
            interval_bounds.push_back(smallest_rel_return + i * interval_width);
        }

        // check number of returns in the individual intervals
        std::ofstream data2("example-data2.dat");
        data2 << "# key lower_interval_boundupper_interval_bound count\n";
        for (auto key : interval_bounds) {
            Real lower_interval_bound = key - interval_width;
            Real upper_interval_bound = key + interval_width;
            multimap_iterator lower = delta_adj_close_rel.lower_bound(lower_interval_bound);
            multimap_iterator upper = delta_adj_close_rel.upper_bound(upper_interval_bound);
            size_t count = std::distance(lower, upper);
            data2 << key << " " << lower_interval_bound << " " << upper_interval_bound << " "
                  << count << "\n";
        }
        data2.close();
        // plot with
        //   gnuplot -e "set terminal png ; set output 'example-data2.png' ; plot 'example-data2.dat' using 1:4 w l"

        return EXIT_SUCCESS;
    } catch (std::exception& e) {
        std::cerr << argv[0] << " caught std::exception:\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
}