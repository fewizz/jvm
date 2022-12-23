#pragma once

#include "class/layout.hpp"

inline optional<_class&> thread_class;
inline layout::position thread_runnable_field_position;

inline reference create_thread();
inline reference create_thread(reference runnable);