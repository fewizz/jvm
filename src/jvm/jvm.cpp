#if 0

d=`realpath ${BASH_SOURCE[0]}`
d=`dirname ${d}`
root=`realpath ${d}/../../`

if clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g \
	-static \
	-nostdinc++ \
	-I ${root}/../core/include \
	-I ${root}/include \
	-o ${root}/build/jvm \
	${d}/jvm.cpp
then
	${root}/build/jvm $@
fi
exit 0
#endif

#include "field_value.hpp"
#include "field.hpp"
#include "class.hpp"
#include "load.hpp"
#include "execute.hpp"

#include <core/c_string.hpp>

int main (int argc, const char** argv) {
	if(argc != 3) {
		fputs("usage: 'class name' 'method name'", stderr);
		return 1;
	}

	_class& cls = load_class(c_string{ argv[1] }.sized());
	field_value fv = execute(cls.find_method(c_string{ argv[2] }));
	if(fv.is<jint>()) {
		printf("%d", fv.get<jint>().value);
	}
}