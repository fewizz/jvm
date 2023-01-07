#include "decl/array.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <class_file/constant.hpp>

#include <posix/abort.hpp>

inline reference _class::get_string(
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

	class_file::constant::string string = string_constant(string_index);
	class_file::constant::utf8 text_utf8 = utf8_constant(string.string_index);

	::reference utf16_string_ref {
		create_string_from_utf8(text_utf8)
	};

	trampoline(string_index) = utf16_string_ref;

	return utf16_string_ref;
}