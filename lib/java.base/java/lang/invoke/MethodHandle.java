package java.lang.invoke;

public abstract class MethodHandle {
	protected long functionPtr_;

	native public MethodType type();

	public final Object invokeExact(Object... args) throws Throwable {
		throw new Error();
	}

	public final Object invoke(Object... args) throws Throwable {
		throw new Error();
	}

}