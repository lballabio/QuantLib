#ifndef quantlib_derived_term_structure_hpp
#define quantlib_derived_term_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <type_traits>
#include <utility>

namespace QuantLib {

    //! Base class for derived term structures that have their own referenceDate.
    template <class Base = YieldTermStructure>
    class DerivedTermStructure : public Base {
      public:
        static_assert(std::is_base_of_v<YieldTermStructure, Base>,
                      "Base must inherit from YieldTermStructure");

        template <class... Args>
        DerivedTermStructure(Handle<YieldTermStructure> originalCurve,
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
        void originalCurveChanged();
    };

    //! Base class for derived term structures that forward referenceDate from
    //! the underlying term structure.
    template <class Base = YieldTermStructure>
    class RelativeDerivedTermStructure : public DerivedTermStructure<Base> {
      public:
        using DerivedTermStructure<Base>::DerivedTermStructure;
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
    inline DerivedTermStructure<Base>::DerivedTermStructure(
        Handle<YieldTermStructure> originalCurve,
        Args&&... args)
    : Base(std::forward<Args>(args)...), originalCurve_(std::move(originalCurve)) {
        this->registerWith(originalCurve_);
        originalCurveChanged();
    }

    template <class Base>
    inline DayCounter DerivedTermStructure<Base>::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    template <class Base>
    inline Date DerivedTermStructure<Base>::maxDate() const {
        return originalCurve_->maxDate();
    }

    template <class Base>
    void DerivedTermStructure<Base>::update() {
        originalCurveChanged();
        Base::update();
    }

    template <class Base>
    inline void DerivedTermStructure<Base>::originalCurveChanged() {
        if (!originalCurve_.empty())
            this->enableExtrapolation(originalCurve_->allowsExtrapolation());
    }

    template <class Base>
    inline const Date& RelativeDerivedTermStructure<Base>::referenceDate() const {
        return this->originalCurve_->referenceDate();
    }

    template <class Base>
    inline Calendar RelativeDerivedTermStructure<Base>::calendar() const {
        return this->originalCurve_->calendar();
    }

    template <class Base>
    inline Natural RelativeDerivedTermStructure<Base>::settlementDays() const {
        return this->originalCurve_->settlementDays();
    }

    template <class Base>
    void RelativeDerivedTermStructure<Base>::update() {
        if (!this->originalCurve_.empty()) {
            DerivedTermStructure<Base>::update();
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
