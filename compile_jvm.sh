root=`dirname $(realpath ${BASH_SOURCE[0]})`

mkdir -p ${root}/build

declare -a additional_args

if [[ $OS != Windows_NT ]]; then
	additional_args+=(-fsanitize=undefined)
	additional_args+=(-fsanitize=memory)
	additional_args+=(-pthreads)
fi

if ! clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g3 \
	-nostdinc++ \
	-fno-exceptions \
	-fno-rtti \
	-fuse-ld=lld \
	-iquote ${root}/src/decl \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/../class-file/include \
	-o ${root}/build/jvm \
	${additional_args[@]} \
	${root}/src/jvm.cpp
then
	exit 1
fi