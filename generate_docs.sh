#!/bin/sh

if [ ! -f doxygen_config ]; then
	echo "File 'doxygen_config' not found:"
	echo "Most likely, you are in the wrong directory!"
	exit 1;
fi

rm -rf doc/programmer_manual/latex
rm -rf doc/programmer_manual/html
doxygen doxygen_config
cd doc/programmer_manual/latex
make
mv refman.pdf ../programmer_manual.pdf
cd ../
rm -rf latex
