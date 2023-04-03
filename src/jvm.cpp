#include "decl/classes.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/thread.hpp"
#include "decl/executable_path.hpp"

#include "impl/impl.hpp"

#include <posix/default_unhandled.cpp>
#include <print/print.hpp>

int main (int argc, const char** argv) {
	if(argc != 2) {
		print::err("usage: 'class name'\n");
		return 1;
	}

	// TODO replace with somethig more reliable
	executable_path = c_string_of_unknown_size{argv[0]}.sized();

	// TODO replace with algo
	lib_path = [] {
		c_string_of_known_size exe = executable_path.get();
		nuint last_slash = exe.size() - 1;

		while(
			last_slash >= 0 &&
			(exe[last_slash] != '\\' && exe[last_slash] != '/')
		){
			--last_slash;
		}
		return c_string_of_known_size{ exe.begin(), last_slash };
	}();

	init_java_lang_object();

	void_class = classes.define_primitive_class(c_string{ "void"    }, 'V');

	auto define_primitive_and_its_array_class = []<basic_range Name>(
		Name&& name, char ch
	) ->_class& {
		_class& component_class = classes.define_primitive_class(name, ch);

		array<char, 2> array_class_name{ '[', ch };
		_class& array_class
			= classes.define_array_class(array_class_name, reference{});

		array_class.component_class(component_class);
		component_class.array_class(array_class);

		return component_class;
	};

	bool_class
		= define_primitive_and_its_array_class(c_string{ "boolean" }, 'Z');
	byte_class
		= define_primitive_and_its_array_class(c_string{ "byte" }, 'B');
	short_class
		= define_primitive_and_its_array_class(c_string{ "short" }, 'S');
	char_class
		= define_primitive_and_its_array_class(c_string{ "char" }, 'C');
	int_class
		= define_primitive_and_its_array_class(c_string{ "int" }, 'I');
	long_class
		= define_primitive_and_its_array_class(c_string{ "long" }, 'J');
	float_class
		= define_primitive_and_its_array_class(c_string{ "float" }, 'F');
	double_class
		= define_primitive_and_its_array_class(c_string{ "double" }, 'D');

	bool_array_class   = bool_class  ->get_array_class();
	byte_array_class   = byte_class  ->get_array_class();
	short_array_class  = short_class ->get_array_class();
	char_array_class   = char_class  ->get_array_class();
	int_array_class    = int_class   ->get_array_class();
	long_array_class   = long_class  ->get_array_class();
	float_array_class  = float_class ->get_array_class();
	double_array_class = double_class->get_array_class();

	init_lib();

	thread = create_thread();

	_class& app_cl_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/AppClassLoader"}
	);
	reference app_cl_ref = create_object(app_cl_class);

	auto main_class_name = c_string{ argv[1] }.sized();

	_class& c = classes.load_class(main_class_name, app_cl_ref);
	method& m = c.declared_static_methods().try_find(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	).if_has_no_value([] {
		print::err("main method is not found\n");
		posix::abort();
	}).get();

	reference args_array = create_array_of(string_class.get(), 0);
	stack.emplace_back(args_array);
	execute(m);

	bool there_was_unhandled_exception = !thrown.is_null();
	on_thread_exit();

	return there_was_unhandled_exception ? -1 : 0;
}