
(defvar ql-file-variables
"/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
")

(defvar ql-license
"
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
")

; skeleton for a new header file---you might want to bind this to some key
(defun ql-new-header ()
  (interactive)
  (insert ql-file-variables)
  (ql-add-license)
  (ql-add-file-description)
  (insert "\n")
  (ql-add-include-guard)
  (ql-add-sample-header)
  (insert "\n")
  (ql-add-namespace))

; skeleton for a new source file---you might want to bind this to some key
(defun ql-new-source ()
  (interactive)
  (insert ql-file-variables)
  (ql-add-license)
  (insert "\n")
  (ql-add-sample-header)
  (insert "\n")
  (ql-add-namespace))

(require 'cc-mode)
(defvar c++-font-lock-extra-types)

; A few types for syntax-highlighting
; from QuantLib:
(setq c++-font-lock-extra-types
      (append c++-font-lock-extra-types
              '("QuantLib"
                "Integer" "BigInteger" "Natural" "BigNatural" "Real" "Decimal"
                "Time" "Rate" "Spread" "DiscountFactor" "Size" "Volatility"
                "Date" "Day" "Month" "Year" "Weekday"
                "TimeUnit" "Frequency" "Compounding" "Period" "DayCounter"
                "Calendar" "BusinessDayConvention"
                "Currency" "ExchangeRate" "Money" "Rounding"
                "InterestRate"
                "History"
                "Handle")))
; from Boost:
(setq c++-font-lock-extra-types
      (append c++-font-lock-extra-types
              '("Boost"
                "shared_ptr" "format")))


; helper functions

(defun ql-add-license ()
  (let ((holder (read-from-minibuffer "Copyright holder? ")))
    (let ((copyright-notice
           (apply 'string (append " Copyright (C) "
                                  (substring (current-time-string) -4)
                                  " "
                                  holder
                                  "\n"
                                  ()))))
      (insert "\n/*\n"
              copyright-notice
              ql-license
              "*/\n\n"))))

(defun ql-add-file-description ()
  (let ((filename (buffer-name))
        (description (read-from-minibuffer "Short file description? ")))
    (insert "/*! \\file " filename "\n"
            "    \\brief " description "\n"
            "*/\n")))

(defun ql-add-include-guard ()
  (let ((guard (read-from-minibuffer "Include guard? ")))
    (insert "#ifndef " guard "\n"
            "#define " guard "\n"
            "\n\n\n"
            "#endif\n"))
  (forward-line -3))

(defun ql-add-sample-header ()
  (insert "#include <ql/qldefines.hpp>\n"))


(defun ql-add-namespace ()
  (insert "namespace QuantLib {\n"
          "\n\n\n"
          "}\n")
  (forward-line -3)
  (c-indent-command))

