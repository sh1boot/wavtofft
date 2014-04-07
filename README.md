wavtofft
========

Quick and dirty command-line FFT tool

Intended usage, something like:
```bash
wavtofft -i input.wav \
    -I "set terminal png ; set output 'output.png' ; set yrange [-96:0]" \
    -p "plot '-' using 1:2 with lines" \
    -P "e"  | gnuplot
```
