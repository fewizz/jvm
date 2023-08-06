package jvm.mh;

import java.lang.invoke.MethodType;

public class Constructor extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Constructor(
		MethodType methodType, boolean isVarargs,
		Class<?> c, short instanceFieldIndex
	) {
		super(methodType, isVarargs, c, instanceFieldIndex);
	}

}