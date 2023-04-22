package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public final class Adapter extends MethodHandle {
	private final MethodHandle original_;

	private Adapter(
		MethodType methodType,
		MethodHandle original
	) {
		super(methodType);
		this.original_ = original;
	}

	@Override
	protected native void invokeExactPtr();

}