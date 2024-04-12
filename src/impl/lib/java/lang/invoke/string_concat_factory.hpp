#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/java/lang/invoke/constant_call_site.hpp"
#include "decl/lib/jvm/mh/string_concat.hpp"

static void init_java_lang_invoke_string_concat_factory() {
	c& scf = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/StringConcatFactory"s
	);
	method& make_concat_with_constants = scf.declared_static_methods().find(
		u8"makeConcatWithConstants"s,
		"("
			"Ljava/lang/invoke/MethodHandles$Lookup;"
			"Ljava/lang/String;"
			"Ljava/lang/invoke/MethodType;"
			"Ljava/lang/String;"
			"[Ljava/lang/Object;"
		")"
		"Ljava/lang/invoke/CallSite;"s
	);

	make_concat_with_constants.native_function(
		(void*)+[](
			native_environment*,
			[[maybe_unused]] object* lookup,
			[[maybe_unused]] j::string* name,
			j::method_type* mt,
			j::string* recipe,
			[[maybe_unused]] object* constants
		) -> object* {
			expected<reference, reference> possible_mh
				= jvm::string_concat::try_create(*mt, *recipe);
			
			if(possible_mh.is_unexpected()) {
				thrown_in_native = possible_mh.move_unexpected();
				return nullptr;
			}

			reference mh_ref = possible_mh.move_expected();
			jvm::string_concat& mh = (jvm::string_concat&) mh_ref.object();

			expected<reference, reference> possible_cs
				= j::constant_call_site::try_create(mh);

			if(possible_cs.is_unexpected()) {
				thrown_in_native = possible_cs.move_unexpected();
				return nullptr;
			}

			reference cs_ref = possible_cs.move_expected();
			return & cs_ref.unsafe_release_without_destroing();
		}
	);
}