#pragma once

#include "./class/layout.hpp"
#include "./class/layout_view_extension.hpp"

#include "./class/constants.hpp"
#include "./class/trampolines.hpp"
#include "./class/bootstrap_methods.hpp"

#include "./class/is_array.hpp"
#include "./class/is_primitive.hpp"

#include "./class/find_by_name_and_descriptor_extension.hpp"

#include "./reference.hpp"
#include "./primitives.hpp"

#include "mutex_attribute_recursive.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>

struct c :
	layout_view_extension< // for static fields
		c,
		declared_static_field_index
	>,
	constants,
	trampolines,
	bootstrap_methods
{
private:
	// mutable
	optional<c&> super_;

	const posix::memory_for_range_of<uint8> bytes_;
	const class_file::access_flags access_flags_;
	const class_file::constant::utf8 name_;
	const posix::memory_for_range_of<const utf8::unit> descriptor_;
	const class_file::constant::utf8 source_file_;

	const posix::memory_for_range_of<c*> declared_interfaces_;

	mutable posix::memory_for_range_of<static_field>
		declared_static_fields_;
	mutable posix::memory_for_range_of<instance_field>
		declared_instance_fields_;

	mutable posix::memory_for_range_of<static_method>
		declared_static_methods_;
	mutable posix::memory_for_range_of<instance_method>
		declared_instance_methods_;

	const posix::memory_for_range_of<instance_field*> instance_fields_;
	const posix::memory_for_range_of<instance_method*> instance_methods_;

	optional<method> initialisation_method_;

	const ::layout instance_layout_;
	const ::layout declared_static_layout_;

	const is_array_class is_array_;
	const is_primitive_class is_primitive_;
	reference defining_loader_;
	body<posix::mutex> mutex_;

	// mutable state:
	optional<c&> array_class_;
	optional<c&> component_class_;
	reference instance_;
	enum initialisation_state {
		not_started,
		pending,
		done
	} initialisation_state_ = not_started;
	posix::memory_for_range_of<uint8> declared_static_fields_data_;

public:

	c(
		constants&&, bootstrap_methods&&,
		posix::memory_for_range_of<uint8> bytes,
		class_file::access_flags,
		class_file::constant::utf8 name,
		posix::memory_for_range_of<utf8::unit> descriptor,
		class_file::constant::utf8 source_file,
		optional<c&> super,
		posix::memory_for_range_of<c*>,
		posix::memory_for_range_of<static_field> declared_static_fields,
		posix::memory_for_range_of<instance_field> declared_instance_fields,
		posix::memory_for_range_of<static_method> declared_static_methods,
		posix::memory_for_range_of<instance_method> declared_instance_methods,
		optional<method> initialisation_method,
		is_array_class,
		is_primitive_class,
		reference loader = {}
	);

	c(c&&) = delete;
	c(const c&) = delete;
	c& operator = (c&&) = delete;
	c& operator = (const c&) = delete;

	inline const instance_field&
	operator[](instance_field_index index) const;
	inline       instance_field&
	operator[](instance_field_index index);

	inline const static_field&
	operator[](declared_static_field_index index) const;
	inline       static_field&
	operator[](declared_static_field_index index);

	inline const instance_method&
	operator[](instance_method_index index) const;
	inline       instance_method&
	operator[](instance_method_index index);

	inline const instance_method&
	operator[](declared_instance_method_index index) const;
	inline       instance_method&
	operator[](declared_instance_method_index index);

	inline const static_method&
	operator[](declared_static_method_index index) const;
	inline       static_method&
	operator[](declared_static_method_index index);

	using constants::operator [];

	class_file::access_flags access_flags() const { return access_flags_; }

	class_file::constant::utf8 name() const {
		return name_;
	}
	span<const utf8::unit, uint16> descriptor() const {
		return descriptor_.as_span();
	}

	span<const utf8::unit> package() const {
		optional<uint16> possible_slash_index
			= name().try_find_index_of_last_satisfying([](utf8::unit ch) {
				return ch == '/';
			});
		if(possible_slash_index.has_value()) {
			uint16 slash_index = possible_slash_index.get();
			return { name().iterator(), slash_index };
		}
		else {
			return {};
		}
	}

	const c& super() const { return super_.get(); }
	      c& super()       { return super_.get(); }
	bool has_super() const { return super_.has_value(); }

	const c* ptr() const & { return this; }
	      c* ptr()       & { return this; }

	class_file::constant::utf8 source_file() const { return source_file_; }
	bool has_source_file() const { return source_file_.iterator() != nullptr; }

	bool is_interface() const { return access_flags_.interface; }
	bool is_public() const { return access_flags_._public; }
	bool is_protected() const { return access_flags_._protected; }
	bool is_private() const { return access_flags_._private; }

	bool has_default_access() const {
		return !is_public() && !is_protected() && !is_private();
	}

	bool is_array() const { return is_array_; }
	bool is_primitive() const { return is_primitive_; }
	bool is_not_primitive() const { return !is_primitive_; }
	bool is_reference() const { return !is_primitive_; }
	bool is(const c& c) const { return c.ptr() == this; }
	bool is_not(const c& c) const { return c.ptr() != this; }

	c& get_array_class();
	c& get_component_class();

	[[nodiscard]] optional<reference> try_initialise_if_need();

	const ::layout& instance_layout() const { return instance_layout_; }
	const ::layout& declared_static_layout() const {
		return declared_static_layout_;
	}

	inline void destruct_declared_static_fields_values();

	// required member functions for layout_view_extension:
	friend layout_view_extension<object, instance_field_index>;

	const ::layout& layout_for_view() const {
		return declared_static_layout_;
	}
	inline uint8* data_for_layout_view() {
		return declared_static_fields_data_.as_span().iterator();
	}
	inline auto fields_view_for_layout_view() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_static_fields_.as_span()),
			declared_static_field_index
		> {
			declared_static_fields_.as_span()
		};
	}
	//

	layout::position instance_field_position(
		instance_field_index index
	) {
		return instance_layout().slot_for_field_index(index).beginning();
	}

	layout::position declared_static_field_position(
		declared_static_field_index index
	) {
		return declared_static_layout().slot_for_field_index(index).beginning();
	}

	template<basic_range Name, basic_range Descriptor>
	layout::position instance_field_position(
		Name&& name, Descriptor&& descriptor
	);

	auto declared_static_fields() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_static_fields_.as_span()),
			declared_static_field_index
		> {
			declared_static_fields_.as_span()
		};
	}
	auto instance_fields() const {
		return find_by_name_and_descriptor_view<
			decltype(instance_fields_.as_span().dereference_view()),
			instance_field_index
		> {
			instance_fields_.as_span().dereference_view()
		};
	}

	auto declared_static_methods() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_static_methods_.as_span()),
			declared_static_method_index
		> {
			declared_static_methods_.as_span()
		};
	}
	auto declared_instance_methods() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_instance_methods_.as_span()),
			declared_instance_method_index
		> {
			declared_instance_methods_.as_span()
		};
	}

	auto declared_instance_fields() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_instance_fields_.as_span()),
			declared_instance_field_index
		> {
			declared_instance_fields_.as_span()
		};
	}
	auto instance_methods() const {
		return find_by_name_and_descriptor_view<
			decltype(instance_methods_.as_span().dereference_view()),
			instance_method_index
		> {
			instance_methods_.as_span().dereference_view()
		};
	}

	auto declared_interfaces() const {
		return declared_interfaces_.transform_view(
			[](storage<c*>& storage) -> c& { return *storage.get(); }
		);
	}

private:
	inline void init_instance();
public:
	::object& object() {
		init_instance();
		return instance_.object();
	}

	::object* object_ptr() {
		init_instance();
		return instance_.object_ptr();
	}

	reference defining_loader() {
		return defining_loader_;
	}

	void array_class(c& c)     { array_class_     = c; }
	void component_class(c& c) { component_class_ = c; }

	[[nodiscard]] expected<reference, reference> try_get_string(
		class_file::constant::string_index string_index
	);

	[[nodiscard]] expected<c&, reference> try_get_resolved_class(
		class_file::constant::class_index string_index
	);

	[[nodiscard]] expected<reference, reference> try_get_resolved_method_handle(
		class_file::constant::method_handle_index index
	);

	[[nodiscard]] expected<reference, reference> try_get_resolved_call_site(
		class_file::constant::invoke_dynamic_index index
	);

	template<typename Verifier>
	[[nodiscard]] expected<method&, reference>
	try_get_resolved_method(
		class_file::constant::method_ref_index ref_index,
		Verifier&& verifier
	);

	template<typename Verifier>
	[[nodiscard]] expected<method&, reference>
	try_get_resolved_interface_method(
		class_file::constant::interface_method_ref_index ref_index,
		Verifier&& verifier
	);

	[[nodiscard]] expected<method&, reference> try_resolve_method(
		class_file::constant::method_ref
	);

	[[nodiscard]] expected<method&, reference> try_resolve_method(
		class_file::constant::method_ref_index index
	) {
		class_file::constant::method_ref method_ref
			= method_ref_constant(index);
		return try_resolve_method(method_ref);
	}

	[[nodiscard]] expected<method&, reference> try_resolve_interface_method(
		class_file::constant::interface_method_ref ref
	);

	[[nodiscard]] expected<method&, reference> try_resolve_interface_method(
		class_file::constant::interface_method_ref_index index
	) {
		class_file::constant::interface_method_ref method_ref
			= interface_method_ref_constant(index);
		return try_resolve_interface_method(method_ref);
	}

	[[nodiscard]] expected<field&, reference> try_resolve_field(
		class_file::constant::field_ref
	);

	[[nodiscard]] expected<field&, reference> try_resolve_field(
		class_file::constant::field_ref_index index
	) {
		class_file::constant::field_ref field_ref
			= field_ref_constant(index);
		return try_resolve_field(field_ref);
	}

	template<typename Verifier>
	[[nodiscard]] expected<field&, reference>
	try_get_resolved_field(
		class_file::constant::field_ref_index ref_index,
		Verifier&& verifier
	);

	/* A maximally-specific superinterface method of a class or interface C for
	   a particular method name and descriptor is any method for which all of
	   the following are true:
	   * The method is declared in a superinterface (direct or indirect) of C.
	   * The method is declared with the specified name and descriptor.
	   * The method has neither its ACC_PRIVATE flag nor its ACC_STATIC
	     flag set.
	   * Where the method is declared in interface I, there exists no other
	     maximally-specific superinterface method of C with the specified name
	     and descriptor that is declared in a subinterface of I. */
	template<typename Name, typename Descriptor, typename Handler>
	void for_each_maximally_specific_super_interface_instance_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	) {
		auto search_for_method = [&](c& c) -> optional<instance_method&> {
			for(instance_method& m : c.declared_instance_methods()) {
				if(
					m.has_name_and_descriptor_equal_to(name, descriptor) &&
					!m.is_private()
				) {
					return m;
				}
			}
			return {};
		};
		struct recursive {
			loop_action operator()(
				Handler&& handler, decltype(search_for_method) search, c& c
			) {
				for(::c& super_i : c.declared_interfaces()) {
					loop_action handlers_action;

					optional<instance_method&> m = search(super_i);

					if(m.has_value()) {
						if constexpr(
							same_as<decltype(handler(m.get())), loop_action>
						) {
							handlers_action = handler(m.get());
						}
						else {
							handlers_action = loop_action::next;
						}
					}
					// search in superinterfaces
					else {
						handlers_action = recursive{}(
							forward<Handler>(handler), search, super_i
						);
					}

					switch (handlers_action) {
						case loop_action::stop: return loop_action::stop;
						case loop_action::next: continue;
					}
				}
				return loop_action::next;
			}
		};
		recursive{}(forward<Handler>(handler), search_for_method, *this);
	}

	template<typename Handler>
	void for_each_super_interface(Handler&& handler) {
		for(c& i : declared_interfaces()) {
			loop_action action = handler(i);
			switch (action) {
				case loop_action::stop: return;
				case loop_action::next: continue;
			}
		}
	}

	bool is_sub_of(c& other) const {
		if(has_super()) {
			const c& s = super();
			if(&s == &other) {
				return true;
			}
			return s.is_sub_of(other);
		}
		return false;
	}

	bool is_implementing(c& other) const {
		for(c& i : declared_interfaces()) {
			if(&i == &other) {
				return true;
			}
			if(i.is_implementing(other)) {
				return true;
			}
		}
		return false;
	}

	template<typename Handler>
	decltype(auto) view_raw_type_non_void(Handler&& handler) {
		if(is_reference()) {
			return handler.template operator()<reference>();
		}
		else {
			if(is(bool_class.get())) {
				return handler.template operator()<bool>();
			} else
			if(is(byte_class.get())) {
				return handler.template operator()<int8>();
			} else
			if(is(short_class.get())) {
				return handler.template operator()<int16>();
			} else
			if(is(char_class.get())) {
				return handler.template operator()<uint16>();
			} else
			if(is(int_class.get())) {
				return handler.template operator()<int32>();
			} else
			if(is(long_class.get())) {
				return handler.template operator()<int64>();
			} else
			if(is(float_class.get())) {
				return handler.template operator()<float>();
			} else
			if(is(double_class.get())) {
				return handler.template operator()<double>();
			} else {
				posix::abort();
			}
		}
	};

	template<typename Handler>
	decltype(auto) view_raw_type(Handler&& handler) {
		if(is(void_class.get())) {
			return handler.template operator()<void_t>();
		} else {
			return view_raw_type_non_void(forward<Handler>(handler));
		}
	}

};