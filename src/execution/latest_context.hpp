#pragma once

#include "./context.hpp"

static thread_local optional<execution_context&> latest_execution_context{};