package jvm.mh;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public class StringConcat extends MethodHandle {
	final String recipe_;

	StringConcat(MethodType mt, String recipe) {
		super(mt, false);
		this.recipe_ = recipe;
	}

	@Override
	protected native void invokeExactPtr();
	
}
