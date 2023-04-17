#pragma once

#include "./class/layout.hpp"
#include "./class/layout_view_extension.hpp"

#include "./class/constants.hpp"
#include "./class/trampolines.hpp"
#include "./class/bootstrap_methods.hpp"

#include "./class/name.hpp"
#include "./class/is_array.hpp"
#include "./class/is_primitive.hpp"

#include "./class/find_by_name_and_descriptor_extension.hpp"

#include "./reference.hpp"

#include "mutex_attribute_recursive.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>

struct _class :
	layout_view_extension< // for static fields
		_class,
		declared_static_field_index
	>,
	constants,
	trampolines,
	bootstrap_methods
{
private:
	// mutable
	optional<_class&> super_;

	const posix::memory_for_range_of<uint8> bytes_;
	const class_file::access_flags access_flags_;
	const this_class_name this_name_;
	const posix::memory_for_range_of<uint8> descriptor_;
	const class_file::constant::utf8 source_file_;

	const posix::memory_for_range_of<_class*> declared_interfaces_;
	posix::memory_for_range_of<field> declared_fields_;
	posix::memory_for_range_of<method> declared_methods_;

	const posix::memory_for_range_of<field*> declared_static_fields_;
	const posix::memory_for_range_of<method*> declared_static_methods_;

	const posix::memory_for_range_of<field*> declared_instance_fields_;
	const posix::memory_for_range_of<method*> declared_instance_methods_;

	const posix::memory_for_range_of<field*> instance_fields_;
	const posix::memory_for_range_of<method*> instance_methods_;

	const ::layout instance_layout_;
	const ::layout declared_static_layout_;

	const is_array_class is_array_;
	const is_primitive_class is_primitive_;
	reference defining_loader_;
	body<posix::mutex> mutex_;

	// mutable state:
	optional<_class&> array_class_;
	optional<_class&> component_class_;
	reference instance_;
	enum initialisation_state {
		not_started,
		pending,
		done
	} initialisation_state_ = not_started;
	posix::memory_for_range_of<uint8> declared_static_fields_data_;

public:

	_class(
		constants&&, bootstrap_methods&&,
		posix::memory_for_range_of<uint8> bytes, class_file::access_flags,
		this_class_name,
		posix::memory_for_range_of<uint8> descriptor,
		class_file::constant::utf8 source_file,
		optional<_class&> super,
		posix::memory_for_range_of<_class*>,
		posix::memory_for_range_of<field>,
		posix::memory_for_range_of<method>,
		is_array_class,
		is_primitive_class,
		reference loader = {}
	);

	_class(_class&&) = delete;
	_class(const _class&) = delete;
	_class& operator = (_class&&) = delete;
	_class& operator = (const _class&) = delete;

	inline const field& operator[](instance_field_index index) const;
	inline       field& operator[](instance_field_index index);

	inline const method& operator[](instance_method_index index) const;
	inline       method& operator[](instance_method_index index);

	inline const method& operator[](declared_instance_method_index index) const;
	inline       method& operator[](declared_instance_method_index index);

	inline const method& operator[](declared_static_method_index index) const;
	inline       method& operator[](declared_static_method_index index);

	using constants::operator [];

	class_file::access_flags access_flags() const { return access_flags_; }

	this_class_name name() const { return this_name_; }
	span<char> descriptor() const {
		return ::span{ (char*) descriptor_.iterator(), descriptor_.size() };
	}

	span<const char> package() const {
		optional<uint16> possible_slash_index
			= name().try_find_index_of_last_satisfying([](char ch) {
				return ch == '/';
			});
		if(possible_slash_index.has_value()) {
			uint16 slash_index = possible_slash_index.get();
			uint16 beginning_offset = slash_index + 1;
			uint16 new_size = name().size() - beginning_offset;
			return { name().iterator() + beginning_offset, new_size };
		}
		else {
			return {};
		}
	}

	const _class& super() const { return super_.get(); }
	      _class& super()       { return super_.get(); }
	bool has_super() const { return super_.has_value(); }

	const _class* ptr() const & { return this; }
	      _class* ptr()       & { return this; }

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
	bool is(const _class& c) const { return c.ptr() == this; }

	_class& get_array_class();
	_class& get_component_class();

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
			decltype(declared_static_fields_.as_span().dereference_view()),
			declared_static_field_index
		> {
			declared_static_fields_.as_span().dereference_view()
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

	auto declared_fields() {
		return find_by_name_and_descriptor_view<
			decltype(declared_fields_.as_span()),
			declared_field_index
		> {
			declared_fields_.as_span()
		};
	}
	auto declared_methods() {
		return find_by_name_and_descriptor_view<
			decltype(declared_methods_.as_span()),
			declared_method_index
		> {
			declared_methods_.as_span()
		};
	}

	auto declared_static_fields() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_static_fields_.as_span().dereference_view()),
			declared_static_field_index
		> {
			declared_static_fields_.as_span().dereference_view()
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
			decltype(declared_static_methods_.as_span().dereference_view()),
			declared_static_method_index
		> {
			declared_static_methods_.as_span().dereference_view()
		};
	}

	auto declared_instance_methods() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_instance_methods_.as_span().dereference_view()),
			declared_instance_method_index
		> {
			declared_instance_methods_.as_span().dereference_view()
		};
	}
	auto declared_instance_fields() const {
		return find_by_name_and_descriptor_view<
			decltype(declared_instance_fields_.as_span().dereference_view()),
			declared_instance_field_index
		> {
			declared_instance_fields_.as_span().dereference_view()
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
			[](storage<_class*>& storage) -> _class& { return *storage.get(); }
		);
	}

	reference instance();

	reference defining_loader() {
		return defining_loader_;
	}

	void array_class(_class& c)     { array_class_     = c; }
	void component_class(_class& c) { component_class_ = c; }

	[[nodiscard]] expected<reference, reference> try_get_string(
		class_file::constant::string_index string_index
	);

	[[nodiscard]] expected<_class&, reference> try_get_resolved_class(
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

	[[nodiscard]] expected<instance_field_index_and_stack_size, reference>
	try_get_resolved_instance_field_index(
		class_file::constant::field_ref_index ref_index
	);

	[[nodiscard]] expected<method&, reference> try_get_resolved_static_method(
		class_file::constant::method_ref_index ref_index
	);

	[[nodiscard]] expected<class_and_declared_static_field_index, reference>
	try_get_static_field_index(
		class_file::constant::field_ref_index ref_index
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
		auto search_for_method = [&](_class& c) -> optional<method&> {
			for(method& m : c.declared_instance_methods()) {
				if(
					has_name_and_descriptor_equal_to{ name, descriptor }(m) &&
					!m.is_private()
				) {
					return m;
				}
			}
			return {};
		};
		struct recursive {
			loop_action operator()(
				Handler&& handler, decltype(search_for_method) search, _class& c
			) {
				for(_class& super_i : c.declared_interfaces()) {
					loop_action handlers_action;

					optional<method&> m = search(super_i);

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
		for(_class& i : declared_interfaces()) {
			loop_action action = handler(i);
			switch (action) {
				case loop_action::stop: return;
				case loop_action::next: continue;
			}
		}
	}

	bool is_sub_of(_class& other) const {
		if(has_super()) {
			const _class& s = super();
			if(&s == &other) {
				return true;
			}
			return s.is_sub_of(other);
		}
		return false;
	}

	bool is_implementing(_class& other) const {
		for(_class& i : declared_interfaces()) {
			if(&i == &other) {
				return true;
			}
			if(i.is_implementing(other)) {
				return true;
			}
		}
		return false;
	}

};