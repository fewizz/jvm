package java.lang.invoke;

import java.util.List;

public final class MethodType {
	final Class<?> return_type_;
	final Class<?>[] parameter_types_;

	private MethodType(Class<?> return_type, Class<?>[] parameter_types) {
		this.return_type_ = return_type;
		this.parameter_types_ = parameter_types;
	}

	public static MethodType methodType(Class<?> rtype, Class<?>[] ptypes) {
		return new MethodType(rtype, ptypes);
	}

	public static MethodType methodType(Class<?> rtype, List<Class<?>> ptypes) {
		return new MethodType(rtype, ptypes.toArray(new Class<?>[]{}));
	}

	public static MethodType methodType(
		Class<?> rtype, Class<?> ptype0, Class<?>... ptypes
	) {
		Class<?>[] ptypes_ = new Class<?>[ptypes.length + 1];
		ptypes_[0] = ptype0;
		System.arraycopy(ptypes, 0, ptypes_, 1, ptypes.length);
		return new MethodType(rtype, ptypes_);
	}

	public static MethodType methodType(Class<?> rtype) {
		return new MethodType(rtype, new Class<?>[]{});
	}

	public static MethodType methodType(Class<?> rtype, Class<?> ptype0) {
		return new MethodType(rtype, new Class<?>[]{ ptype0 });
	}

}