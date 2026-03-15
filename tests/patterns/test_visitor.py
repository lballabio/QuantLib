"""Tests for AcyclicVisitor and Visitor pattern.

Hand-written (no upstream C++ test file).
"""

from quantlib.patterns.visitor import AcyclicVisitor, Visitor


class Bond:
    def accept(self, visitor):
        if isinstance(visitor, Visitor):
            visitor.visit(self)


class Option:
    def accept(self, visitor):
        if isinstance(visitor, Visitor):
            visitor.visit(self)


class PricingVisitor(AcyclicVisitor, Visitor):
    def __init__(self):
        self.visited = []

    def visit(self, obj):
        self.visited.append(type(obj).__name__)


class TestVisitor:
    def test_acyclic_visitor_isinstance(self):
        """AcyclicVisitor type check works."""
        v = PricingVisitor()
        assert isinstance(v, AcyclicVisitor)
        assert isinstance(v, Visitor)

    def test_visitor_dispatch(self):
        """Basic accept/visit double dispatch works."""
        v = PricingVisitor()
        bond = Bond()
        option = Option()

        bond.accept(v)
        option.accept(v)

        assert v.visited == ["Bond", "Option"]

    def test_non_visitor_ignored(self):
        """accept() is a no-op if visitor is not a Visitor instance."""

        class NotAVisitor:
            pass

        bond = Bond()
        nv = NotAVisitor()
        bond.accept(nv)  # Should not raise
