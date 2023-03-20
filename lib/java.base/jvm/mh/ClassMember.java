package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public abstract class ClassMember extends MethodHandle {
	protected final Class<?> class_;
	protected final int memberIndex_;

	protected ClassMember(
		MethodType methodType, Class<?> c, int memberIndex
	) {
		super(methodType);
		this.class_ = c;
		this.memberIndex_ = memberIndex;
	}

}
