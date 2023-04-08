package jvm.mh;

import java.lang.invoke.MethodType;

public class Setter extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Setter(
		MethodType methodType, Class<?> c, short instanceFieldIndex
	) {
		super(methodType, c, instanceFieldIndex);
	}

}