package jvm.mh;

import java.lang.invoke.MethodType;

public class Getter extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Getter(
		MethodType methodType, Class<?> c, int instanceFieldIndex
	) {
		super(methodType, c, instanceFieldIndex);
	}

}
