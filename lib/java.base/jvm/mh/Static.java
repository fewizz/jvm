package jvm.mh;

import java.lang.invoke.MethodType;

public class Static extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Static(
		MethodType methodType, boolean isVarargs,
		Class<?> c, short declaredStaticMethodIndex
	) {
		super(methodType, isVarargs, c, declaredStaticMethodIndex);
	}

}