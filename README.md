# bgl-kmb
Implementation of Kou, Markowsky and Berman algorithm (KMB 1981) using the Boost Graph Library

### Requires
1. g++-4.8
2. boost 1.55

### Build
1. Set BOOST_INSTALL_ROOT to your boost installation folder
2. Run `make` to build the release version
3. Run `make debug` to build the debug version
4. Run `make test` to test

### Scripts
1. brite2dot.sh : Converts BRITE format files to dot. Run with `./brite2dot.sh input.brite`
2. inet2dot.sh : Converts inet format files to dot. Run with `./inet2dot.sh inputfile`
2. gtitm2dot.sh : Converts gtitm format files to dot. Run with `./gtitm-alt2dot.sh inputfile`. Convert the `.gb` file to the alteraate format using `sgb2alt` provided in the gt-itm distribution.

### Author
Snehasish Kumar
