#pragma once

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <optional.hpp>

static optional<c&> lambda_meta_factory_class;

static void init_java_lang_invoke_lambda_meta_factory() {

	lambda_meta_factory_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/LambdaMetaFactory" }
	);

	lambda_meta_factory_class->declared_static_methods().find(
		c_string{ u8"metafactory" },
		c_string{ u8"("
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
			[[maybe_unused]] o<jl::object>* caller,
			[[maybe_unused]] o<jl::object>* interfaceMethodName,
			[[maybe_unused]] o<jl::object>* factoryType,
			[[maybe_unused]] o<jl::object>* interfaceMethodType,
			[[maybe_unused]] o<jl::object>* implementation,
			[[maybe_unused]] o<jl::object>* dynamicMethodType
		) {
			posix::abort();
		}
	);

}