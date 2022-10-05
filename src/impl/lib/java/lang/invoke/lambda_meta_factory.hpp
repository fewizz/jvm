#pragma once

#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

#include <optional.hpp>

static optional<_class&> lambda_meta_factory_class;

static void init_java_lang_invoke_lambda_meta_factory() {

	lambda_meta_factory_class =
		classes.find_or_load(c_string{ "java/lang/invoke/LambdaMetaFactory" });

	lambda_meta_factory_class->declared_methods().find(
		c_string{ "metafactory" },
		c_string{ "("
			"Ljava/lang/invoke/MethodHandles$Lookup;"
			"Ljava/lang/String;"
			"Ljava/lang/invoke/MethodType;"
			"Ljava/lang/invoke/MethodType;"
			"Ljava/lang/invoke/MethodHandle;"
			"Ljava/lang/invoke/MethodType;"
		")"
		"Ljava/lang/invoke/CallSite;"
		}
	).native_function(
		(void*) (void (*)(
			native_interface_environment*,
			object*, object*, object*, object*, object*, object*)
		)
		[](
			native_interface_environment*,
			[[maybe_unused]] object* caller,
			[[maybe_unused]] object* interfaceMethodName,
			[[maybe_unused]] object* factoryType,
			[[maybe_unused]] object* interfaceMethodType,
			[[maybe_unused]] object* implementation,
			[[maybe_unused]] object* dynamicMethodType
		) {
			abort();
		}
	);

}