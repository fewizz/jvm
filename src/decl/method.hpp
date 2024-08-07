#pragma once

#include "./method/code.hpp"
#include "./parameters_count.hpp"
#include "./class/member.hpp"
#include "./descriptor.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/descriptor/type.hpp>
#include <class_file/descriptor/method.hpp>
#include <class_file/attribute/code/exception_handler.hpp>
#include <class_file/attribute/line_numbers/reader.hpp>

#include <span.hpp>
#include <variant.hpp>
#include <optional.hpp>
#include <list.hpp>
#include <c_string.hpp>

#include <posix/memory.hpp>

struct native_function_ptr {
private:
	void* value_;
public:
	native_function_ptr(void* value) : value_{ value } {}

	operator void* () const { return value_; }
};

using code_or_native_function_ptr =
	variant<code, native_function_ptr>;

using exception_handlers = list<
	posix::memory<class_file::attribute::code::exception_handler>
>;

struct method : class_member {
private:

	code_or_native_function_ptr code_;
	initialised<posix::memory<class_file::attribute::code::exception_handler>>
		exception_handlers_;
	uint8 parameters_stack_size_ = 0;
	initialised<posix::memory<one_of_descriptor_parameter_types>>
		parameter_types_;
	one_of_descriptor_return_types return_type_{ class_file::v{} };
	initialised<posix::memory<tuple<uint16, class_file::line_number>>>
		line_numbers_;

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::utf8             name,
		class_file::constant::utf8             descriptor,
		code_or_native_function_ptr            code,
		initialised<
			posix::memory<class_file::attribute::code::exception_handler>
		>&& exception_handlers,
		initialised<
			posix::memory<tuple<uint16, class_file::line_number>>
		>&& line_numbers
	) :
		class_member       { access_flags, name, descriptor },
		code_              { code                           },
		exception_handlers_{ move(exception_handlers)       },
		line_numbers_      { move(line_numbers)             }
	{
		uint8 parameters_count =
			class_file::method_descriptor::reader {
				descriptor.iterator()
			}.try_read_parameters_count(
				[]([[maybe_unused]] auto err) { posix::abort(); }
			).get();

		list parameters_types_list =
			posix::allocate<one_of_descriptor_parameter_types>(
				parameters_count
			);

		auto return_type_reader =
			class_file::method_descriptor::reader {
				descriptor.iterator()
			}.try_read_parameter_types_and_get_return_type_reader(
				overloaded {
					[&]<class_file::parameter_descriptor_type Type> {
						parameters_types_list.emplace_back(Type{});
					},
					[&](class_file::reference_type auto ref_type) {
						parameters_types_list.emplace_back(ref_type);
					}
				},
				[](auto) { posix::abort(); }
			).get();

		return_type_reader.try_read_and_get_advanced_iterator(
			overloaded {
				[&]<class_file::return_descriptor_type Type> {
					return_type_ = Type{};
				},
				[&](class_file::reference_type auto ref_type) {
					return_type_ = ref_type;
				}
			},
			[](auto) -> one_of_descriptor_return_types { posix::abort(); }
		);

		if(!access_flags._static) {
			++parameters_stack_size_; // this
		}
		for(one_of_descriptor_parameter_types t : parameters_types_list) {
			t.view([&]<typename Type>(Type) {
				parameters_stack_size_ += descriptor_type_stack_size<Type>;
			});
		}
		parameter_types_ = move(parameters_types_list).as_initialised();
	}

	uint8 parameters_stack_size() const { return parameters_stack_size_; }

	auto& parameter_types() const {
		return parameter_types_;
	}

	parameters_count parameters_count() {
		auto count = range_size(parameter_types_);
		return ::parameters_count{ (uint8) count };
	}

	code code() const { return code_.get_same_as<::code>(); }

	auto& exception_handlers() {
		return exception_handlers_;
	}

	auto& line_numbers() {
		return line_numbers_;
	}

	bool is_native() const { return access_flags_.native; }
	bool is_synchronized() const { return access_flags_.super_or_synchronized; }
	bool is_abstract() const { return access_flags_.abstract; }
	bool is_varargs() const { return access_flags_.varargs; }

	bool has_variable_number_of_arguments() const {
		return access_flags_.varargs;
	}

	bool native_function_is_loaded() const {
		return code_.get_same_as<native_function_ptr>() != nullptr;
	}

	void native_function(native_function_ptr function) {
		if(!code_.is_same_as<native_function_ptr>()) {
			posix::abort();
		}
		code_ = function;
	}

	native_function_ptr native_function() {
		return code_.get_same_as<native_function_ptr>();
	}

	one_of_descriptor_return_types return_type() const {
		return return_type_;
	}

	bool is_void() const {
		return return_type_.is_same_as<class_file::v>();
	}

	bool is_instance_initialisation() const;

	bool is_class_initialisation() const;

	bool is_signature_polymorphic() const;

	bool can_override(method&) const;

};

struct static_method : method {
};

struct instance_method : method {
	using method::method;
	instance_method(method&& m) : method{ move(m) } {}
};

#include <types.hpp>

template<typename T0, typename T1>
requires (
	type_is_lvalue_reference<T0> &&
	type_is_lvalue_reference<T1> &&
	derived_from<remove_reference<T0>, ::method> &&
	derived_from<remove_reference<T1>, ::method>
)
struct __types::common::result<T0, T1> {
	using type = method&;
};