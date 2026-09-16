
POSITIONAL_ARGS=()
models=()
configs=()
destination_folder=""

help_desc() {
    echo "usage: ./generate_ctx.sh -m|--models <model1 model2 ...> -c|--configs <config1 config2 ...> [-d|--destination <destination_folder>]"
}

if [[ $# -eq 0 ]]; then
    help_desc
    exit 1
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        -m|--models)
            shift
            while [[ $# -gt 0 && "$1" != -* ]]; do
                models+=("$1")
                shift
            done
            ;;

        -c|--configs)
            shift
            while [[ $# -gt 0 && "$1" != -* ]]; do
                configs+=("$1")
                shift
            done
            ;;

        -d|--destination)
            if [[ $# -lt 2 ]]; then
                echo "Missing argument for $1"
                exit 1
            fi
            destination_folder="$2"
            shift 2
            ;;

        -*|--*)
            echo "Unknown option $1"
            exit 1
            ;;

        *)
            POSITIONAL_ARGS+=("$1")
            shift
            ;;
    esac
done

echo "=============="
echo "Models to compile: $(echo $models)"
echo "Configuration files to use: $(echo $configs)"
echo "Compiled models will be saved at: $destination_folder"
echo "=============="

for model in "${models[@]}"
do
    for config in "${configs[@]}"
    do
        echo "--------------"
        echo "Generating $model with $config...."
        ./get_opt_model -m $model --ctx_dest $destination_folder -i Ort -o '{"EP_name": "QNN"}' --ort_json_ep_options $config
        echo "--------------"
    done
done
