package java.lang.invoke;

public abstract class MethodHandle {
	private final MethodType methodType_;
	private final boolean isVarargs_;

	protected MethodHandle(MethodType methodType, boolean isVarargs) {
		this.methodType_ = methodType;
		this.isVarargs_ = isVarargs;
	}

	protected abstract void invokeExactPtr();
	protected native void invokePtr();

	public MethodType type() {
		return methodType_;
	}

	public native final Object invokeExact(Object... args) throws Throwable;

	public native final Object invoke(Object... args) throws Throwable;

	public native Object invokeWithArguments(Object... args) throws Throwable;

	public native MethodHandle asType(MethodType newType);

	public final boolean isVarargsCollector() {
		return this.isVarargs_;
	}

}