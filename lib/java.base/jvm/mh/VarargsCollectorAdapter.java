package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;
import java.lang.invoke.WrongMethodTypeException;

public class VarargsCollectorAdapter extends MethodHandle {

	private final MethodHandle original_;

	private native boolean check();

	private VarargsCollectorAdapter(
		MethodType methodType,
		MethodHandle original
	) {
		super(methodType, true);
		this.original_ = original;
		if(!check()) {
			throw new WrongMethodTypeException();
		}
	}

	@Override
	protected native void invokeExactPtr();

}