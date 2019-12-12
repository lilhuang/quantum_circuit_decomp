import subprocess

samples = [
    ["rand30-25","Samples/4regRand30Node26-circ.qasm",25],
    ["rand20-12","Samples/4regRand20Node1-p1.qasm",12],
    ["rand20-19","Samples/4regRand20Node1-p1.qasm",19],
]

csv_data = ["genetic_score,genetic_time,metis_score,metis_time\n"]
for title,fname,qubits in samples:
    cmd = ["./release/compare_partitionings",str(qubits),fname]
    print(" ".join(cmd))
    output = subprocess.check_output(cmd)
    output = output.decode("utf-8")
    csv_data.append(output)

all_out = "".join(csv_data)
print(all_out)
#with open("comparison_output.csv") as out:
#    out.write(all_out)
