package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public abstract class ClassMember extends MethodHandle {
	protected final Class<?> class_;
	protected final short memberIndex_;

	protected ClassMember(
		MethodType methodType, boolean isVarargs, Class<?> c, short memberIndex
	) {
		super(methodType, isVarargs);
		this.class_ = c;
		this.memberIndex_ = memberIndex;
	}

}