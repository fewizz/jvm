package jvm.mh;

import java.lang.invoke.MethodType;

public class StaticGetter extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private StaticGetter(
		MethodType methodType, Class<?> c, short staticFieldIndex
	) {
		super(methodType, false, c, staticFieldIndex);
	}

}