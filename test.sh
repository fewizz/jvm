root=`dirname ${BASH_SOURCE[0]}`

test() {
	echo "test \"$1\""

	if ! javac ${root}/test/$1.java
	then
		echo "compilation error"
		exit 1
	fi

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

for testfile in `cd 'test' && find -name "*.java" -type f && cd ..`; do
	test "`dirname $testfile`/`basename $testfile .java`"
done