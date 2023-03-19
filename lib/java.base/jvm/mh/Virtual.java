package jvm.mh;

public class Virtual extends ClassMember {

	private static native long functionPtr();

	private Virtual(Class<?> c, int declaredStaticMethodIndex) {
		super(c, declaredStaticMethodIndex);
		this.functionPtr_ = functionPtr();
	}
	
}
