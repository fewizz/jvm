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
			= classes.define_array_class(array_class_name, nullptr);

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

	auto on_exit = [&](optional<reference> possible_thrown){
		on_thread_exit(possible_thrown);
		return possible_thrown.has_value() ? -1 : 0;
	};

	expected<reference, reference> possible_thread = try_create_thread();
	if(possible_thread.is_unexpected()) {
		return on_exit(move(possible_thread.get_unexpected()));
	}

	_class& app_cl_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/AppClassLoader"}
	);
	expected<reference, reference> possible_app_cl_ref
		= try_create_object(app_cl_class);
	if(possible_app_cl_ref.is_unexpected()) {
		return on_exit(move(possible_app_cl_ref.get_unexpected()));
	}

	reference app_cl_ref = move(possible_app_cl_ref.get_expected());

	auto main_class_name = c_string{ argv[1] }.sized();

	expected<_class&, reference> possible_c
		= classes.try_load_class(main_class_name, app_cl_ref.object_ptr());

	if(possible_c.is_unexpected()) {
		return on_exit(move(possible_c.get_unexpected()));
	}

	_class& c = possible_c.get_expected();
	method& m = c.declared_static_methods().try_find(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	).if_has_no_value([] {
		print::err("main method is not found\n");
		posix::abort();
	}).get();

	expected<reference, reference> possible_args_array
		= try_create_array_of(string_class.get(), 0);

	if(possible_args_array.is_unexpected()) {
		return on_exit(move(possible_args_array.get_unexpected()));
	}

	reference args_array = move(possible_args_array.get_unexpected());

	stack.emplace_back(args_array);

	return on_exit(try_execute(m));
}