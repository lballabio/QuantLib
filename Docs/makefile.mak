#
# Makefile for documentation under Borland C++
#

# Primary target:
# Complete documentation
Docs: MakeDocs MakePDF

# Generate docs with doxygen
MakeDocs:
	@doxygen doxygen.cfg

# Compile LaTeX sources to PDF
MakePDF::
	@cd latex
	@pdflatex refman
	@makeindex refman.idx
	@pdflatex refman
	@cd..

