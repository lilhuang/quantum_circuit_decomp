# quantum_circuit_decomp

### Compiling instructions

This is packaged to be a simple cmake project. Like any other cmake project, something like:

    mkdir build
    cd build
    cmake ..
    make -j8

Should compile the project. This requires CMake 3.5 or above, a c compile and GNU make, but should work for many other configurations.

### Running instructions

You can run a tool that visualizes the output decomposition by

    view_quant_circ <number_partitions> <circuit_filename>

For example,

    ./build/view_quant_circ 4 Samples/4regRand20Node5-p1.qasm
