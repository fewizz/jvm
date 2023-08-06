package java.lang.invoke;

public abstract class CallSite {
	private final MethodType type_;

	protected CallSite(MethodType type) {
		this.type_ = type;
	}

	public MethodType type() {
		return type_;
	}

	public abstract MethodHandle getTarget();
	public abstract void setTarget(MethodHandle newTarget);

}