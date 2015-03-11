#!/bin/bash

DATA_DIR='data'
PLOT_IMAGE_FILE='data/plot.png'
TEMP_GNUPLOT_SCRIPT='plot.gps'

TITLE='Convolution'
X_LABEL='Taille du filtre (pixels)'
Y_LABEL="Temps d exécution (secondes)"

rm -f $TEMP_GNUPLOT_SCRIPT;

echo 'set terminal png nocrop enhanced size 1024,1024 font "arial, 12"' >> $TEMP_GNUPLOT_SCRIPT
echo 'set key bmargin left horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid' >> $TEMP_GNUPLOT_SCRIPT

echo "set output '$PLOT_IMAGE_FILE'" >> $TEMP_GNUPLOT_SCRIPT

echo 'set samples 800, 800' >> $TEMP_GNUPLOT_SCRIPT

echo "set title '$TITLE'" >> $TEMP_GNUPLOT_SCRIPT
echo 'set title  font ",20" norotate' >> $TEMP_GNUPLOT_SCRIPT
echo "set xlabel '$X_LABEL'" >> $TEMP_GNUPLOT_SCRIPT
echo "set ylabel '$Y_LABEL'" >> $TEMP_GNUPLOT_SCRIPT

echo  >> $TEMP_GNUPLOT_SCRIPT
echo 'x = 0.0' >> $TEMP_GNUPLOT_SCRIPT
echo  >> $TEMP_GNUPLOT_SCRIPT

PLOT_COMMAND="plot"
for file in "$DATA_DIR"/*.dat
do
    if [ -f "$file" ]
    then
	PLOT_COMMAND+=" '";
	PLOT_COMMAND+=$file;
	PLOT_COMMAND+="' with lines,";
    fi
done
echo $PLOT_COMMAND >> $TEMP_GNUPLOT_SCRIPT

gnuplot < $TEMP_GNUPLOT_SCRIPT
rm -f $TEMP_GNUPLOT_SCRIPT

eog $PLOT_IMAGE_FILE
