root=`dirname $(realpath ${BASH_SOURCE[0]})`

mkdir -p ${root}/build

declare -a additional_args

while getopts "i" o; do
	case $o in
		i) additional_args+=(-DPRINT_EXECUTION_INFO=1);;
	esac
done

if [[ $OS != Windows_NT ]]; then
	``
	additional_args+=(-fsanitize=address)
	#additional_args+=(-fsanitize=undefined)
	#additional_args+=(-fsanitize=memory)
fi

if ! clang++ \
	-fverbose-asm \
	-ferror-limit=64 \
	-std=c++2b \
	-Wall -Wextra \
	-g \
	-O0 \
	-nostdinc++ \
	-nostdinc \
	-fno-exceptions \
	-fno-rtti \
	-fuse-ld=lld \
	-iquote ${root}/src/decl \
	-iquote ${root}/src \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/../class-file/include \
	-I ${root}/../posix-wrapper/include \
	-I ${root}/../print/include \
	-o ${root}/build/jvm \
	-pthread \
	${additional_args[@]} \
	${root}/src/jvm.cpp
then
	exit 1
fi