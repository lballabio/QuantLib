"""Acyclic Visitor pattern for type-safe double dispatch."""

from abc import ABC, abstractmethod


class AcyclicVisitor:
    """Base class for all visitors.

    Concrete visitors inherit from both AcyclicVisitor and Visitor,
    implementing visit() for each type they handle.
    """

    pass


class Visitor(ABC):
    """Mixin indicating a visitor can handle a specific type.

    In C++ this is a template: ``Visitor<T>``. In Python, concrete visitors
    inherit from AcyclicVisitor and implement visit() methods for each
    type they handle. The visited object calls ``visitor.visit(self)``.
    """

    @abstractmethod
    def visit(self, obj):
        """Visit the given object. Must be overridden by concrete visitors."""
