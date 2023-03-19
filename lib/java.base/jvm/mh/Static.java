package jvm.mh;

public class Static extends ClassMember {

	private static native long functionPtr();

	private Static(Class<?> c, int declaredStaticMethodIndex) {
		super(c, declaredStaticMethodIndex);
		this.functionPtr_ = functionPtr();
	}

}
