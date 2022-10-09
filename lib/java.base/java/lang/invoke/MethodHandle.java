package java.lang.invoke;

public class MethodHandle {
	private long member_;
	private byte kind_;

	private MethodHandle() {}

	native public MethodType type();

	public final native Object invokeExact(Object... args) throws Throwable;
	public final native Object invoke(Object... args) throws Throwable;

}