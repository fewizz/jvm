package jvm.mh;

import java.lang.invoke.MethodHandle;

public class ClassMember extends MethodHandle {
	protected final Class<?> class_;
	protected final int memberIndex_;

	protected ClassMember(Class<?> c, int memberIndex) {
		this.class_ = c;
		this.memberIndex_ = memberIndex;
	}

}
