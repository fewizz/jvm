package jvm.mh;

public class Getter extends ClassMember {

	private static native long functionPtr();

	private Getter(Class<?> c, int instanceFieldIndex) {
		super(c, instanceFieldIndex);
		this.functionPtr_ = functionPtr();
	}

}
