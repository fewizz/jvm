package java.lang.invoke;

import java.util.List;

public final class MethodType {
	final Class<?>[] parameterTypes_;
	final Class<?> returnType_;
	final byte[] descriptorUTF8_;

	private static native byte[] descriptorUTF8(
		Class<?>[] parameterTypes, Class<?> returnType
	);

	private MethodType(Class<?> returnType, Class<?>[] parameterTypes) {
		this.parameterTypes_ = parameterTypes;
		this.returnType_ = returnType;
		this.descriptorUTF8_ = descriptorUTF8(parameterTypes, returnType);
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