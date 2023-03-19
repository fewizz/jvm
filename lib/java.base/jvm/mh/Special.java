package jvm.mh;

public class Special extends ClassMember {

	private static native long functionPtr();

	private Special(Class<?> c, int instanceFieldIndex) {
		super(c, instanceFieldIndex);
		this.functionPtr_ = functionPtr();
	}


}
