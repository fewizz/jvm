package jvm.mh;

import java.lang.invoke.MethodType;

public class Special extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Special(
		MethodType methodType, boolean isVarargs,
		Class<?> c, short instanceFieldIndex
	) {
		super(methodType, isVarargs, c, instanceFieldIndex);
	}

}
