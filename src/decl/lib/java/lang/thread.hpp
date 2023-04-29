#pragma once

#include "class/layout.hpp"

inline optional<c&> thread_class;
inline layout::position thread_runnable_field_position;

inline expected<reference, reference> try_create_thread();
inline expected<reference, reference> try_create_thread(reference runnable);