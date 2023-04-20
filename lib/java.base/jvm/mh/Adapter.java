import java.lang.invoke.MethodHandle;

public final class Adapter extends MethodHandle {
	private final MethodHandle original_;

	private Adapter(
		MethodType methodType,
		MethodHandle original,
	) {
		super(methodType);
		this.original_ = original;
	}

	@Override
	protected native void invokeExactPtr();

}