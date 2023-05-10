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
	executable_path = c_string{ (const utf8::unit*) argv[0] }.sized();

	// TODO replace with algo
	lib_path = [] {
		auto exe = executable_path.get();
		nuint last_slash = exe.size() - 1;

		while(
			last_slash >= 0 &&
			(exe[last_slash] != '\\' && exe[last_slash] != '/')
		){
			--last_slash;
		}
		return c_string_of_known_size<utf8::unit>{ exe.begin(), last_slash };
	}();

	init_java_lang_object();

	void_class = classes.define_primitive_class(c_string{ u8"void" }, 'V');

	auto define_primitive_and_its_array_class = []<basic_range Name>(
		Name&& name, utf8::unit ch
	) -> c& {
		 c& component_class = classes.define_primitive_class(name, ch);

		array<utf8::unit, 2> array_class_name{ u8'[', ch };
		c& array_class
			= classes.define_array_class(array_class_name, nullptr);

		array_class.component_class(component_class);
		component_class.array_class(array_class);

		return component_class;
	};

	bool_class
		= define_primitive_and_its_array_class(c_string{ u8"boolean" }, 'Z');
	byte_class
		= define_primitive_and_its_array_class(c_string{ u8"byte" }, 'B');
	short_class
		= define_primitive_and_its_array_class(c_string{ u8"short" }, 'S');
	char_class
		= define_primitive_and_its_array_class(c_string{ u8"char" }, 'C');
	int_class
		= define_primitive_and_its_array_class(c_string{ u8"int" }, 'I');
	long_class
		= define_primitive_and_its_array_class(c_string{ u8"long" }, 'J');
	float_class
		= define_primitive_and_its_array_class(c_string{ u8"float" }, 'F');
	double_class
		= define_primitive_and_its_array_class(c_string{ u8"double" }, 'D');

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
		return on_exit(possible_thread.move_unexpected());
	}

	c& app_cl_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/AppClassLoader" }
	);

	optional<reference> possible_exception
		= app_cl_class.try_initialise_if_need();
	if(possible_exception.has_value()) {
		posix::abort();
	}

	optional<field&> possible_app_cl_instance_field
		= try_resolve_field0(
			app_cl_class,
			c_string{ u8"INSTANCE" },
			c_string{ u8"Ljava/lang/ClassLoader;" }
		);
	if(possible_app_cl_instance_field.has_no_value()) {
		posix::abort();
	}

	static_field& app_cl_instance_field
		= (static_field&) possible_app_cl_instance_field.get();

	reference& app_cl_ref =
		get_static_resolved<reference>(app_cl_instance_field);

	auto main_class_name = c_string{ (const utf8::unit*) argv[1] }.sized();

	expected<c&, reference> possible_c
		= classes.try_load_class(main_class_name, app_cl_ref.object_ptr());

	if(possible_c.is_unexpected()) {
		return on_exit(possible_c.move_unexpected());
	}

	c& c = possible_c.get_expected();
	method& m = c.declared_static_methods().try_find(
		c_string{ u8"main" },
		c_string{ u8"([Ljava/lang/String;)V" }
	).if_has_no_value([] {
		print::err("main method is not found\n");
		posix::abort();
	}).get();

	expected<reference, reference> possible_args_array
		= try_create_array_of(string_class.get(), 0);

	if(possible_args_array.is_unexpected()) {
		return on_exit(possible_args_array.move_unexpected());
	}

	reference args_array = possible_args_array.move_unexpected();

	stack.emplace_back(args_array);

	return on_exit(try_execute(m));
}