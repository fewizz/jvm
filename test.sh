root=`dirname ${BASH_SOURCE[0]}`

test() {
	echo "compiling $1.java..."
	if ! javac ${root}/test/$1.java
	then
		echo "compilation error"
		exit 1
	fi

	echo "running $1..."

	pushd ${root}/test > /dev/null
	../build/jvm $1
	result=$?
	popd > /dev/null
	if [[ ${result} == 0 ]] ; then
		echo "success"
	else
		echo "failure"
		exit ${result}
	fi
}

if [ ! -z $1 ]; then
	test $1
	exit
fi

pushd ${root}/test > /dev/null
files=`find * -name "*.java" -type f`
popd > /dev/null

for testfile in $files; do
	test ${testfile%.*}
done