package java.lang.invoke;

public abstract class CallSite {

	public MethodType type() {
		return getTarget().type();
	}

	public abstract MethodHandle getTarget();
	public abstract void setTarget(MethodHandle newTarget);

}