package java.lang.invoke;

public abstract class MethodHandle {
	private final MethodType methodType_;

	protected MethodHandle(MethodType methodType) {
		this.methodType_ = methodType;
	}

	protected abstract void invokeExactPtr();

	public MethodType type() {
		return methodType_;
	}

	public native final Object invokeExact(Object... args) throws Throwable;

	public native final Object invoke(Object... args) throws Throwable;

	public native MethodHandle asType(MethodType newType);

}