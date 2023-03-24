package jvm.mh;

import java.lang.invoke.MethodType;

public class Special extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Special(
		MethodType methodType, Class<?> c, short instanceFieldIndex
	) {
		super(methodType, c, instanceFieldIndex);
	}


}
