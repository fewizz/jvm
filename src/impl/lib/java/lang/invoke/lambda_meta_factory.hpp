#pragma once

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <optional.hpp>

static optional<c&> lambda_meta_factory_class;

static void init_java_lang_invoke_lambda_meta_factory() {

	lambda_meta_factory_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/LambdaMetaFactory" }
	);

	lambda_meta_factory_class->declared_static_methods().find(
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
		(void*)+[](
			native_environment*,
			[[maybe_unused]] object* caller,
			[[maybe_unused]] object* interfaceMethodName,
			[[maybe_unused]] object* factoryType,
			[[maybe_unused]] object* interfaceMethodType,
			[[maybe_unused]] object* implementation,
			[[maybe_unused]] object* dynamicMethodType
		) {
			posix::abort();
		}
	);

}