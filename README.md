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

    view_quant_circ <number_qubits> <circuit_filename>

For example,

    ./build/view_quant_circ 15 Samples/4regRand20Node5-p1.qasm


### Output format

Take a very simple circuit in the qasm format (file in Samples/catStateEightQubits.qasm):

    8
    H 0
    CNOT 0 1
    CNOT 1 2
    CNOT 2 3
    CNOT 3 4
    CNOT 4 5
    CNOT 5 6
    CNOT 6 7

After you run the program on it `./build/view_quant_circ 2 Samples/catStateEightQubits.qasm`, you get

    #number of subcircuits: 2
    #circuit 0
    E E E E
    4
    H 0
    CNOT 0 1
    CNOT 1 2
    CNOT 2 3
    F0 F1 F2 R0
    #circuit 1
    E E E E R0
    5
    CNOT 4 0
    CNOT 0 1
    CNOT 1 2
    CNOT 2 3
    F4 F5 F6 F7 F3
    communi cost: 1
    num qubits: 5

How to read this? Here is a commented version

    #number of subcircuits: 2
    #circuit 0
    E E E E         # each circuit starts with its input qubit description. E means the qubit is set to zero
    4               # now a regular .qasm circuit description, starting with the number of qubits
    H 0
    CNOT 0 1
    CNOT 1 2
    CNOT 2 3
    F0 F1 F2 R0     # output qubit description. R0 means that its output feeds into another circuit
    #circuit 1      # circuit number 2 starts
    E E E E R0      # R0 means that it takes input from the circuit output labeled as R0
    5
    CNOT 4 0
    CNOT 0 1
    CNOT 1 2
    CNOT 2 3
    F4 F5 F6 F7 F3
    communi cost: 1
    num qubits: 5   #F0 means that this output should be interpreted as the 0th qubit of the original circuit.
