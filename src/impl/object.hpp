#include "decl/object.hpp"

#include "decl/class.hpp"
#include "decl/field.hpp"
#include "decl/array.hpp"

#include "execution/info.hpp"

#include <posix/memory.hpp>
#include <posix/io.hpp>

decltype(auto) object::view_ptr(instance_field_position index, auto&& handler) {
	layout::slot s = class_->layout().slot_for_field_index(index);
	uint8* ptr = data_.as_span().iterator() + s.beginning();
	
	field& f = class_->instance_fields()[index];
	return f.type.view([&]<typename Type>(Type) -> decltype(auto) {
		if constexpr(
			same_as<Type, class_file::object> ||
			same_as<Type, class_file::array>
		) {
			return handler((reference*) ptr);
		}
		else if constexpr(same_as<Type, class_file::d>) {
			return handler((double*) ptr);
		}
		else if constexpr(same_as<Type, class_file::j>) {
			return handler((int64*) ptr);
		}
		else if constexpr(same_as<Type, class_file::i>) {
			return handler((int32*) ptr);
		}
		else if constexpr(same_as<Type, class_file::f>) {
			return handler((float*) ptr);
		}
		else if constexpr(same_as<Type, class_file::c>) {
			return handler((uint16*) ptr);
		}
		else if constexpr(same_as<Type, class_file::s>) {
			return handler((int16*) ptr);
		}
		else if constexpr(same_as<Type, class_file::b>) {
			return handler((int8*) ptr);
		}
		else if constexpr(same_as<Type, class_file::z>) {
			return handler((bool*) ptr);
		}
		else {
			posix::abort();
		}
	});
}

decltype(auto) object::view(instance_field_position index, auto&& handler) {
	return view_ptr(index, [&]<typename Type>(Type* e) -> decltype(auto) {
		Type& ref = *e;
		return handler(ref);
	});
}

template<typename Type>
decltype(auto) object::view(instance_field_position position, auto&& handler) {
	static_assert(
		same_as<Type, reference> ||
		same_as<Type, int64> || same_as<Type, int32> ||
		same_as<Type, double> || same_as<Type, float> ||
		same_as<Type, int16> || same_as<Type, uint16> ||
		same_as<Type, int8> || same_as<Type, bool>
	);
	uint8* ptr = data_.as_span().iterator() + position;
	Type& e = * (Type*) ptr;
	return handler(e);
}

inline object::object(::_class& c) :
	class_{ c },
	data_ {
		posix::allocate_memory_for<uint8>(c.layout().total_size())
	}
{
	if(info) {
		tabs();
		print("constructing object @");
		print_hex((nuint)this);
		print(" of type ");
		auto name = _class().name();
		print(name);
		print("\n");
	}

	c.instance_fields().for_each_index([&](nuint field_index) {
		auto position =
			_class().layout().slot_for_field_index(field_index).position();
		view_ptr(position, []<typename Type>(Type* ptr) {
			new (ptr) Type();
		});
	});
}

inline object::~object() {
	if(info) {
		tabs();
		print("destructing object @");
		print_hex((nuint)this);
		print(" of type ");
		auto name = _class().name();
		print(name);
		print("\n");
	}

	if(_class().is_array()) {
		uint8* data = array_data<uint8>(*this);
		if(_class().get_component_class().is_not_primitive()) {
			for(nuint x = array_length(*this); x > 0; --x) {
				((reference*) data)[x - 1].~reference();
			}
		}
		posix::free_raw_memory(data);
	}

	class_->instance_fields().for_each_index([&](nuint field_index) {
		auto position =
			_class().layout().slot_for_field_index(field_index).position();
		view(position, []<typename Type>(Type& e) {
			e.~Type();
		});
	});
}

inline void object::on_reference_added() {
	++references_;
	if(info) {
		tabs();
		print("added reference to object @");
		print_hex((nuint)this);
		print(", now there is ");
		print(references_);
		print("\n");
	}
}

inline void object::on_reference_removed() {
	if(references_ == 0) {
		posix::std_err.write_from(
			c_string{"removing reference on object without references\n"}
		);
		posix::abort();
	}
	--references_;
	if(info) {
		tabs();
		print("removed reference to object @");
		print_hex((nuint)this);
		print(", now there is ");
		print(references_);
		print("\n");
	}
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;
		this->~object();
		posix::free_raw_memory(ptr_to_this);
	}
}

inline void object::unsafe_decrease_reference_count_without_destroing() {
	if(references_ == 0) {
		posix::std_err.write_from(c_string {
			"'unsafe_decrease_reference_count_without_destroing'"
			" on object without references\n"
		});
		posix::abort();
	}
	--references_;
}

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* ptr = posix::allocate_raw_memory_of<object>(1).iterator();
	new(ptr) object(c);
	return { *ptr };
}