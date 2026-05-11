source ./setup.sh

single_run_length=${1:-5}
repeat=${2:-5}
model=${3:-static_cheby1_ord2_low.onnx}

for ep in $(echo WebGPU XNNPACK CPU);
do
    for i in $(seq $repeat);
    do
        ./filtered -m ${model} -i Ort -f 150 -r $single_run_length -p -o "{\"EP_name\": \"${ep}\" }" -P ${ep}_${i}_latency.npy
    done
done