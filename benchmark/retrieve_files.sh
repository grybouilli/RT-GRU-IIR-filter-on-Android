source ./setup.sh

destination=${1:-bench_latency}
single_run_length=${2:-5}
repeat=${3:-5}

for ep in $(echo WebGPU XNNPACK CPU);
do
    for i in $(seq $repeat);
    do
        adb pull /data/local/tmp/${ep}_${i}_latency.npy ${destination}/${ep}_${i}_latency.npy
    done
done

for backend in $(echo cpu gpu htp);
do
    for i in $(seq $repeat);
    do
        adb pull /data/local/tmp/onnxqnn64/qnn_${backend}_${i}_latency.npy ${destination}/qnn_${backend}_${i}_latency.npy
    done
done