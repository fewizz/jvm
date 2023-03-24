package jvm.mh;

import java.lang.invoke.MethodType;

public class Getter extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Getter(
		MethodType methodType, Class<?> c, short instanceFieldIndex
	) {
		super(methodType, c, instanceFieldIndex);
	}

}
