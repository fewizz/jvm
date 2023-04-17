#pragma once

#include "decl/class.hpp"
#include "decl/class/member.hpp"

/* Access control is applied during resolution (§5.4.3) to ensure that a
   reference to a class, interface, field, or method is permitted. Access
   control succeeds if a specified class, interface, field, or method is
   accessible to the referring class or interface. */

[[nodiscard]] inline optional<reference>
access_control(_class& d, _class& c);

[[nodiscard]] inline optional<reference>
access_control(_class& d, class_member& r);