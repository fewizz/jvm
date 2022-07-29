root=`dirname $(realpath ${BASH_SOURCE[0]})`

mkdir -p ${root}/build
if ! javac \
	--system none \
	--module-source-path ${root}/lib \
	-d ${root}/build \
	`find ${root}/lib/ -type f`
then
	exit 1
fi