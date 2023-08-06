package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;
import java.lang.invoke.WrongMethodTypeException;

public final class InvokeAdapter extends MethodHandle {
	private final MethodHandle original_;

	private native boolean check();

	private InvokeAdapter(
		MethodType methodType,
		boolean isVarargs,
		MethodHandle original
	) {
		super(methodType, isVarargs);
		this.original_ = original;
		if(!check()) {
			throw new WrongMethodTypeException();
		}
	}

	@Override
	protected native void invokeExactPtr();

}