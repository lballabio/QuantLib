#
# Makefile for documentation
#

# Primary target:
# Complete documentation
Docs: RunDoxygen CompileLaTeX

# Generate docs with doxygen
RunDoxygen:
	@doxygen doxygen.cfg

# Compile LaTeX sources to PDF
CompileLaTeX::
	@cd latex
	@pdflatex refman
	@makeindex refman.idx
	@pdflatex refman
	@cd..

