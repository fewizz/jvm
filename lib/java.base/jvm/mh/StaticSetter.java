package jvm.mh;

import java.lang.invoke.MethodType;

public class StaticSetter extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private StaticSetter(
		MethodType methodType, Class<?> c, short staticFieldIndex
	) {
		super(methodType, false, c, staticFieldIndex);
	}

}