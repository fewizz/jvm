package java.lang.invoke;

public class ConstantCallSite extends CallSite {

	private final MethodHandle target_;

	public ConstantCallSite(MethodHandle target) {
		this.target_ = target;
	}

	@Override
	public final MethodHandle getTarget() {
		return this.target_;
	}

	@Override
	public final void setTarget(MethodHandle ignore) {
		throw new UnsupportedOperationException();
	}

}