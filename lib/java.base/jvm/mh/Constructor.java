package jvm.mh;

public class Constructor extends ClassMember {

	private static native long functionPtr();

	private Constructor(Class<?> c, int instanceFieldIndex) {
		super(c, instanceFieldIndex);
		this.functionPtr_ = functionPtr();
	}

}