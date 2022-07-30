root=`dirname $(realpath ${BASH_SOURCE[0]})`

mkdir -p ${root}/build

declare -a additional_params

if [[ $OS != Windows_NT ]]; then
	additional_parameters+=(-fsanitize=undefined)
	additional_parameters+=(-fsanitize=memory)
	additional_parameters+=(-pthreads)
fi

if ! clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g3 \
	-nostdinc++ \
	-fno-exceptions \
	-fno-rtti \
	-fuse-ld=lld \
	-iquote ${root}/src \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/../class-file/include \
	-o ${root}/build/jvm \
	${additional_parameters[@]} \
	${root}/src/jvm.cpp
then
	exit 1
fi