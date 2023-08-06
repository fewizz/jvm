#include "decl/array.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <class_file/constant.hpp>

#include <posix/abort.hpp>

inline expected<reference, reference> c::try_get_string(
	class_file::constant::string_index string_index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto& t = trampoline(string_index); t.has_value()) {
		if(!t.is_same_as<::reference>()) {
			posix::abort();
		}
		return t.get_same_as<::reference>();
	}

	class_file::constant::string string = (*this)[string_index];
	class_file::constant::utf8 text_utf8 = (*this)[string.utf8_constant_index];

	expected<::reference, ::reference> possible_utf16_string_ref
		= try_create_string_from_utf8(text_utf8);
	
	if(possible_utf16_string_ref.is_unexpected()) {
		return unexpected{ possible_utf16_string_ref.move_unexpected() };
	}
	reference utf16_string_ref = possible_utf16_string_ref.move_expected();

	trampoline(string_index) = utf16_string_ref;

	return utf16_string_ref;
}