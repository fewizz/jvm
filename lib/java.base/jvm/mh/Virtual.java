package jvm.mh;

import java.lang.invoke.MethodType;

public class Virtual extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Virtual(
		MethodType methodType, boolean isVarargs,
		Class<?> c, short declaredStaticMethodIndex
	) {
		super(methodType, isVarargs, c, declaredStaticMethodIndex);
	}

}
