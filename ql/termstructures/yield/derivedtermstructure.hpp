#ifndef quantlib_derived_yield_term_structure_hpp
#define quantlib_derived_yield_term_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <type_traits>
#include <utility>

namespace QuantLib {

    //! Base class for derived term structures that have their own referenceDate.
    template <class Base = YieldTermStructure>
    class DerivedYieldTermStructure : public Base {
      public:
        static_assert(std::is_base_of_v<YieldTermStructure, Base>,
                      "Base must inherit from YieldTermStructure");

        template <class... Args>
        DerivedYieldTermStructure(Handle<YieldTermStructure> originalCurve,
                                  Args&&... args);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Date maxDate() const override;
        //@}
      protected:
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        Handle<YieldTermStructure> originalCurve_;
      private:
        void setupExtrapolation();
    };

    //! Base class for derived term structures that forward referenceDate from
    //! the underlying term structure.
    template <class Base = YieldTermStructure>
    class RelativeDerivedYieldTermStructure : public DerivedYieldTermStructure<Base> {
      public:
        using DerivedYieldTermStructure<Base>::DerivedYieldTermStructure;
        //! \name YieldTermStructure interface
        //@{
        const Date& referenceDate() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        //@}
      protected:
        //! \name Observer interface
        //@{
        void update() override;
        //@}
    };


    // inline definitions

    template <class Base>
    template <class... Args>
    inline DerivedYieldTermStructure<Base>::DerivedYieldTermStructure(
        Handle<YieldTermStructure> originalCurve,
        Args&&... args)
    : Base(std::forward<Args>(args)...), originalCurve_(std::move(originalCurve)) {
        this->registerWith(originalCurve_);
        setupExtrapolation();
    }

    template <class Base>
    inline DayCounter DerivedYieldTermStructure<Base>::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    template <class Base>
    inline Date DerivedYieldTermStructure<Base>::maxDate() const {
        return originalCurve_->maxDate();
    }

    template <class Base>
    void DerivedYieldTermStructure<Base>::update() {
        setupExtrapolation();
        Base::update();
    }

    template <class Base>
    inline void DerivedYieldTermStructure<Base>::setupExtrapolation() {
        if (!originalCurve_.empty())
            this->enableExtrapolation(originalCurve_->allowsExtrapolation());
    }

    template <class Base>
    inline const Date& RelativeDerivedYieldTermStructure<Base>::referenceDate() const {
        return this->originalCurve_->referenceDate();
    }

    template <class Base>
    inline Calendar RelativeDerivedYieldTermStructure<Base>::calendar() const {
        return this->originalCurve_->calendar();
    }

    template <class Base>
    inline Natural RelativeDerivedYieldTermStructure<Base>::settlementDays() const {
        return this->originalCurve_->settlementDays();
    }

    template <class Base>
    void RelativeDerivedYieldTermStructure<Base>::update() {
        if (!this->originalCurve_.empty()) {
            DerivedYieldTermStructure<Base>::update();
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            // NOLINTNEXTLINE(bugprone-parent-virtual-call)
            TermStructure::update();
        }
    }

}

#endif
