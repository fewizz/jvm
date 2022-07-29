root=`dirname $(realpath ${BASH_SOURCE[0]})`

mkdir -p ${root}/build

if ! clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-fsanitize=undefined \
	-fsanitize=memory \
	-g3 \
	-nostdinc++ \
	-fno-exceptions \
	-pthreads \
	-fno-rtti \
	-fuse-ld=lld \
	-iquote ${root}/src \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/../class-file/include \
	-o ${root}/build/jvm \
	${root}/src/jvm.cpp
then
	exit 1
fi