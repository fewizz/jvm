package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;
import java.lang.invoke.WrongMethodTypeException;

public final class Adapter extends MethodHandle {
	private final MethodHandle original_;

	private native boolean check();

	private Adapter(
		MethodType methodType,
		MethodHandle original
	) {
		super(methodType);
		this.original_ = original;
		if(!check()) {
			throw new WrongMethodTypeException();
		}
	}

	@Override
	protected native void invokeExactPtr();

}