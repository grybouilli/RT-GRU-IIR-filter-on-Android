
POSITIONAL_ARGS=()
model=""
input_audios=""
buffer_size=256
destination_folder=""
hw="htp"
help_desc() {
    echo "usage: ./inferences_on_dataset.sh -m|--model <model.onnx> -d|--dataset <dataset_folder> -o|--output_folder <output_folder> --hw [htp,cpu] -b|--buffer_size"
}

if [[ $# -eq 0 ]]; then
    help_desc
    exit 1
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        -m|--model)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            model="$2"
            shift 2
            ;;

        -d|--dataset)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            input_audios="$2"
            shift 2
            ;;

        -o|--output_folder)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            destination_folder="$2"
            shift 2
            ;;
        -b|--buffer_size)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            buffer_size="$2"
            shift 2
            ;;
        --hw)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            hw="$2"
            shift 2
            ;;

        -*|--*)
            echo "Unknown option $1"
            exit 1
            ;;
    esac
done

if [[ "$destination_folder" == "" ]]; then
    destination_folder="aux_${model##*/}"
    destination_folder="${destination_folder%.*}"
fi

echo "=============="
echo "Model to run: $(echo $model)"
echo "Dataset folder to use: $(echo $input_audios)"
echo "Latencies and output audio will put in: $destination_folder"
echo "=============="

out_latency_dir=$destination_folder/latencies
out_audio_dir=$destination_folder/outputs

mkdir -p $out_latency_dir
mkdir -p $out_audio_dir

for audio in $(ls $input_audios)
do
    ./offline_inference_2sec --file $input_audios/$audio -i Ort \
    -m $model -o '{"EP_name": "QNN", "EP_options": {"backend_type":"'"${hw}"'", "htp_arch":"73", "htp_graph_finalization_optimization_mode":"3", "enable_htp_fp16_precision":"1"}}' \
    --dsp_sample_rate 8000 -b $buffer_size --aux_destination_folder $destination_folder
done
