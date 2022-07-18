root=`dirname ${BASH_SOURCE[0]}`

echo "compiling jvm"
if ! bash ${root}/compile_jvm.sh; then exit 1; fi

echo "compiling lib"
if ! bash ${root}/compile_lib.sh; then exit 1; fi