#pragma once

#include "./stack_entry.hpp"
#include  "./context.hpp"

#include "method/with_class/decl.hpp"

#include <core/expected.hpp>
#include <core/meta/elements/optional.hpp>

using arguments_container = span<stack_entry, uint16>;
using args_container = arguments_container;

inline thread_local optional<execution_context&> latest_execution_ctx{};

static expected<stack_entry, reference> execute(
	method_with_class m, arguments_container args
);