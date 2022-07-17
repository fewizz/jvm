d=`realpath ${BASH_SOURCE[0]}`
d=`dirname ${d}`
root=`realpath ${d}`

if ! clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g3 \
	-static \
	-nostdinc++ \
	-fno-exceptions \
	-fno-rtti \
	-fuse-ld=lld \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/../class-file/include \
	-I ${root}/include \
	-o ${root}/build/jvm \
	${root}/src/jvm.cpp
then
	exit 1
fi