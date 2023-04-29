package jvm.mh;

import java.lang.invoke.MethodType;

public class Virtual extends ClassMember {

	@Override
	protected native void invokeExactPtr();

	private Virtual(
		MethodType methodType, Class<?> c, short declaredStaticMethodIndex
	) {
		super(methodType, c, declaredStaticMethodIndex);
	}

}
